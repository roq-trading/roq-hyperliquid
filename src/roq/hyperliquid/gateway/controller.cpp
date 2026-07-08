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
    result.try_emplace(static_cast<std::string_view>(account.name), std::move(obj));
  }
  return result;
}

template <typename R>
R create_web_socket(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto obj = std::make_unique<WebSocket>(gateway, context, ++stream_id, account, shared);
    result.try_emplace(static_cast<std::string_view>(account.name), std::move(obj));
  }
  return result;
}

template <typename R>
R create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto obj = std::make_unique<OrderEntry>(gateway, context, ++stream_id, account, shared);
    result.try_emplace(static_cast<std::string_view>(account.name), std::move(obj));
  }
  return result;
}

template <typename R>
R create_drop_copy(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto obj = std::make_unique<DropCopy>(gateway, context, ++stream_id, account, shared);
    result.try_emplace(static_cast<std::string_view>(account.name), std::move(obj));
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
    : dispatcher_{dispatcher}, accounts_{create_accounts<decltype(accounts_)>(settings, config)}, context_{context}, shared_{dispatcher, settings},
      rest_{*this, context_, ++stream_id_, shared_} {
  // must be delayed until we have symbols
  assert(std::empty(web_socket_));
  assert(std::empty(order_entry_));
}

// server::Handler

void Controller::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  assert(std::empty(market_data_));
  dispatch(event);
}

void Controller::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Controller::operator()(Event<Timer> const &event) {
  dispatch(event);
}

void Controller::operator()(Event<Control> const &event) {
  auto &[message_info, control] = event;
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

void Controller::operator()(Event<Connected> const &) {
}

void Controller::operator()(Event<Disconnected> const &) {
}

void Controller::operator()(Event<Subscribe> const &event) {
  auto &[message_info, subscribe] = event;
  std::vector<Symbol> symbols;
  for (auto &item : subscribe.symbols) {
    if (shared_.all_symbols.emplace(item).second) {
      symbols.emplace_back(item);
    } else {
      log::warn(R"(*** DUPLICATE SUBSCRIPTION *** (symbol="{}")"sv, item);
    }
  }
  auto symbols_update = Rest::SymbolsUpdate{
      .symbols = symbols,
  };
  (*this)(symbols_update);
}

uint16_t Controller::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  if (shared_.settings.ws_api) {
    return get_web_socket(event.value.account)(event, order, ref_data, request_id);
  } else {
    return get_order_entry(event.value.account)(event, order, ref_data, request_id);
  }
}

uint16_t Controller::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  if (shared_.settings.ws_api) {
    return get_web_socket(event.value.account)(event, order, ref_data, request_id, previous_request_id);
  } else {
    return get_order_entry(event.value.account)(event, order, ref_data, request_id, previous_request_id);
  }
}

uint16_t Controller::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  if (shared_.settings.ws_api) {
    return get_web_socket(event.value.account)(event, order, ref_data, request_id, previous_request_id);
  } else {
    return get_order_entry(event.value.account)(event, order, ref_data, request_id, previous_request_id);
  }
}

uint16_t Controller::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  if (shared_.settings.ws_api) {
    return get_web_socket(event.value.account)(event, request_id);
  } else {
    return get_order_entry(event.value.account)(event, request_id);
  }
}

uint16_t Controller::operator()(Event<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(Event<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Controller::operator()(metrics::Writer &writer) const {
  dispatch_helper(*this, writer);
}

// Rest::Handler

void Controller::operator()(Rest::SymbolsUpdate &symbols_update) {
  auto [size, start_from] = shared_.symbols(symbols_update.symbols);
  ensure_symbol_slices(size);
  for (auto &item : market_data_) {
    (*item).subscribe(start_from);
  }
  // delayed creation of order-entry due to need for symbols
  if (!std::empty(accounts_)) {
    if (shared_.settings.ws_api) {
      if (std::empty(web_socket_)) {
        web_socket_ = create_web_socket<decltype(web_socket_)>(*this, context_, stream_id_, accounts_, shared_);
        MessageInfo message_info;
        Start start;
        for (auto &[_, item] : web_socket_) {
          create_event_and_dispatch(*item, message_info, start);
        }
      }
    } else {
      if (std::empty(order_entry_)) {
        order_entry_ = create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, accounts_, shared_);
        MessageInfo message_info;
        Start start;
        for (auto &[_, item] : order_entry_) {
          create_event_and_dispatch(*item, message_info, start);
        }
      }
    }
    if (std::empty(drop_copy_)) {
      drop_copy_ = create_drop_copy<decltype(drop_copy_)>(*this, context_, stream_id_, accounts_, shared_);
      MessageInfo message_info;
      Start start;
      for (auto &[_, item] : drop_copy_) {
        create_event_and_dispatch(*item, message_info, start);
      }
    }
  }
}

// utilities

void Controller::ensure_symbol_slices(size_t size) {
  while (std::size(market_data_) < size) {
    log::info("Create market-data (user-stream)"sv);
    auto market_data = std::make_unique<MarketData>(*this, context_, ++stream_id_, shared_, std::size(market_data_));
    MessageInfo message_info;
    Start start;
    create_event_and_dispatch(*market_data, message_info, start);
    market_data_.emplace_back(std::move(market_data));
  }
}

template <typename... Args>
void Controller::dispatch(Args &&...args) {
  dispatch_helper(*this, std::forward<Args>(args)...);
}

template <typename... Args>
void Controller::dispatch_helper(auto &self, Args &&...args) {
  auto helper = [&](auto &target) { target(std::forward<Args>(args)...); };
  helper(self.rest_);
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

WebSocket &Controller::get_web_socket(std::string_view const &account) {
  auto iter = web_socket_.find(account);
  if (iter == std::end(web_socket_)) [[unlikely]] {
    throw RuntimeError{R"(Unknown account="{}")"sv, account};
  }
  return *(*iter).second;
}

OrderEntry &Controller::get_order_entry(std::string_view const &account) {
  auto iter = order_entry_.find(account);
  if (iter == std::end(order_entry_)) [[unlikely]] {
    throw RuntimeError{R"(Unknown account="{}")"sv, account};
  }
  return *(*iter).second;
}

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
