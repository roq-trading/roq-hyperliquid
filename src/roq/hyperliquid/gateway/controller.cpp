/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/gateway/controller.hpp"

#include "roq/logging.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/hyperliquid/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace gateway {

// === HELPERS ===

namespace {
template <typename R>
R create_accounts(auto &settings, auto &config) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, account] : config.accounts) {
    auto obj = std::make_unique<Account>(settings, config, account.name);
    result.try_emplace(account.name, std::move(obj));
  }
  return result;
}

template <typename R>
R create_requests(auto &config, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, account] : config.accounts) {
    auto obj = std::make_unique<Request>(shared);
    result.try_emplace(account.name, std::move(obj));
  }
  return result;
}

Request &get_request(auto &requests, auto &account) {
  auto iter = requests.find(account);
  if (iter == std::end(requests)) {
    log::fatal("Unexpected"sv);
  }
  return *(*iter).second;
}

auto create_rest(auto &gateway, auto &context, auto &stream_id, auto &shared) {
  return std::make_unique<Rest>(gateway, context, ++stream_id, shared);
}

template <typename R>
R create_web_socket(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto obj = std::make_unique<WebSocket>(gateway, context, ++stream_id, account, shared);
    result.try_emplace(account.name, std::move(obj));
  }
  return result;
}

template <typename R>
R create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared, auto &requests) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto &request = get_request(requests, account.name);
    auto obj = std::make_unique<OrderEntry>(gateway, context, ++stream_id, account, shared, request);
    result.try_emplace(account.name, std::move(obj));
  }
  return result;
}

template <typename R>
R create_drop_copy(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared, auto &requests) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto &request = get_request(requests, account.name);
    auto obj = std::make_unique<DropCopy>(gateway, context, ++stream_id, account, shared, request);
    result.try_emplace(account.name, std::move(obj));
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

std::unique_ptr<server::Handler> Controller::create(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context) {
  return std::make_unique<Controller>(dispatcher, settings, config, context);
}

uint8_t Controller::parse_api(Settings const &) {
  return {};
}

Controller::Controller(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_{dispatcher}, context_{context}, shared_{dispatcher, settings}, accounts_{create_accounts<decltype(accounts_)>(settings, config)},
      requests_{create_requests<decltype(requests_)>(config, shared_)}, rest_{create_rest(*this, context_, stream_id_, shared_)} {
  // must be delayed until we have symbols
  assert(std::empty(web_socket_));
  assert(std::empty(order_entry_));
}

// server::Handler

void Controller::operator()(Trace<Start> const &event) {
  log::info("Starting..."sv);
  assert(std::empty(market_data_));
  dispatch(event);
}

void Controller::operator()(Trace<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Controller::operator()(Trace<Timer> const &event) {
  dispatch(event);
}

void Controller::operator()(Trace<Connected> const &, [[maybe_unused]] uint8_t user_id) {
}

void Controller::operator()(Trace<Disconnected> const &, [[maybe_unused]] uint8_t user_id) {
}

void Controller::operator()(Trace<Subscribe> const &event) {
  auto &[trace_info, subscribe] = event;
  std::vector<Symbol> symbols;
  for (auto &item : subscribe.symbols) {
    if (shared_.all_symbols.emplace(item).second) {
      symbols.emplace_back(item);
    } else {
      log::warn(R"(*** DUPLICATE SUBSCRIPTION *** (symbol="{}")"sv, item);
    }
  }
  auto symbols_update = SymbolsUpdate{
      .symbols = symbols,
  };
  create_trace_and_dispatch_2(trace_info, symbols_update);
}

void Controller::operator()(Trace<Control> const &event) {
  auto &[trace_info, control] = event;
  switch (control.action) {
    using enum Action;
    case UNDEFINED:
      assert(false);
      break;
    case ENABLE:
      dispatcher_(State::ENABLED);
      break;
    case DISABLE:
      dispatcher_(State::DISABLED);
      break;
  }
}

void Controller::operator()(Trace<server::oms::OrderActionBatchBegin> const &event) {
  dispatch_order_action(event);
}

void Controller::operator()(Trace<server::oms::OrderActionBatchCommit> const &event) {
  dispatch_order_action(event);
}

void Controller::operator()(Trace<server::oms::OrderActionBatchRollback> const &event) {
  dispatch_order_action(event);
}

uint16_t Controller::operator()(
    Trace<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  return dispatch_order_action(event, order, ref_data, request_id);
}

uint16_t Controller::operator()(
    Trace<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  return dispatch_order_action(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Controller::operator()(
    Trace<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  return dispatch_order_action(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Controller::operator()(Trace<CancelAllOrders> const &event, std::string_view const &request_id) {
  return dispatch_order_action(event, request_id);
}

uint16_t Controller::operator()(Trace<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(Trace<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Controller::operator()(metrics::Writer &writer) const {
  dispatch_helper(*this, writer);
}

// Rest::Handler

void Controller::operator()(Trace<SymbolsUpdate> const &event) {
  auto &[trace_info, symbols_update] = event;
  auto [size, start_from] = shared_.symbols(symbols_update.symbols);
  ensure_symbol_slices(trace_info, size);
  for (auto &item : market_data_) {
    dynamic_cast<server::MarketDataStream &>(*item).subscribe(start_from);
  }
  // delayed creation of order-entry due to need for symbols
  if (!std::empty(accounts_)) {
    if (!shared_.settings.misc.disable_ws_api) {
      if (std::empty(web_socket_)) {
        web_socket_ = create_web_socket<decltype(web_socket_)>(*this, context_, stream_id_, accounts_, shared_);
        Start start;
        for (auto &[_, item] : web_socket_) {
          create_trace_and_dispatch(dynamic_cast<server::Stream &>(*item), trace_info, start);
        }
      }
    }
    if (std::empty(order_entry_)) {
      order_entry_ = create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, accounts_, shared_, requests_);
      Start start;
      for (auto &[_, item] : order_entry_) {
        create_trace_and_dispatch(dynamic_cast<server::Stream &>(*item), trace_info, start);
      }
    }
    if (std::empty(drop_copy_)) {
      drop_copy_ = create_drop_copy<decltype(drop_copy_)>(*this, context_, stream_id_, accounts_, shared_, requests_);
      Start start;
      for (auto &[_, item] : drop_copy_) {
        create_trace_and_dispatch(dynamic_cast<server::Stream &>(*item), trace_info, start);
      }
    }
  }
}

// utilities

void Controller::ensure_symbol_slices(TraceInfo const &trace_info, size_t size) {
  while (std::size(market_data_) < size) {
    log::info("Create market-data (user-stream)"sv);
    auto obj = std::make_unique<MarketData>(*this, context_, ++stream_id_, shared_, std::size(market_data_));
    Start start;
    create_trace_and_dispatch(dynamic_cast<server::Stream &>(*obj), trace_info, start);
    market_data_.emplace_back(std::move(obj));
  }
}

template <typename... Args>
void Controller::dispatch(Args &&...args) {
  dispatch_helper(*this, std::forward<Args>(args)...);
}

template <typename... Args>
void Controller::dispatch_helper(auto &self, Args &&...args) {
  auto helper = [&](auto &target) {
    if constexpr (std::is_const_v<decltype(self)>) {
      dynamic_cast<server::Stream const &>(target)(std::forward<Args>(args)...);
    } else {
      dynamic_cast<server::Stream &>(target)(std::forward<Args>(args)...);
    }
  };
  helper(*self.rest_);
  for (auto &item : self.market_data_) {
    helper(*item);
  }
  for (auto &[_, item] : self.order_entry_) {
    helper(*item);
  }
  for (auto &[_, item] : self.web_socket_) {
    helper(*item);
  }
  for (auto &[_, item] : self.drop_copy_) {
    helper(*item);
  }
}

template <typename T, typename... Args>
uint16_t Controller::dispatch_order_action(Trace<T> const &event, Args &&...args) {
  auto &[trace_info, value] = event;
  if (shared_.settings.misc.disable_ws_api) {
    auto iter = order_entry_.find(value.account);
    if (iter == std::end(order_entry_)) [[unlikely]] {
      throw RuntimeError{R"(Unknown account="{}")"sv, value.account};
    }
    auto &stream = dynamic_cast<server::OrderActionStream &>(*(*iter).second);
    stream(event, args...);
    return stream.stream_id;
  } else {
    auto iter = web_socket_.find(value.account);
    if (iter == std::end(web_socket_)) [[unlikely]] {
      throw RuntimeError{R"(Unknown account="{}")"sv, value.account};
    }
    auto &stream = dynamic_cast<server::OrderActionStream &>(*(*iter).second);
    stream(event, args...);
    return stream.stream_id;
  }
}

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
