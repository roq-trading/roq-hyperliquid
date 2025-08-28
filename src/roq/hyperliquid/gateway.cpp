/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/hyperliquid/gateway.hpp"

#include <algorithm>
#include <cctype>
#include <limits>

#include "roq/logging.hpp"

#include "roq/clock.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/hyperliquid/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {

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
}  // namespace

// === IMPLEMENTATION ===

Gateway::Gateway(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_{dispatcher}, accounts_{create_accounts<decltype(accounts_)>(settings, config)}, context_{context}, shared_{dispatcher, settings},
      rest_{*this, context_, ++stream_id_, shared_}, market_data_{*this, context_, ++stream_id_, shared_, {}} {
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  dispatch(event);
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  dispatch(event);
}

void Gateway::operator()(Event<Control> const &event) {
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

void Gateway::operator()(Event<Connected> const &) {
}

void Gateway::operator()(Event<Disconnected> const &) {
}

void Gateway::operator()(Trace<StreamStatus> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ExternalLatency> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ReferenceData> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketStatus> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TopOfBook> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) {
  auto callback = []([[maybe_unused]] auto &market_by_price) {};
  dispatcher_(event, is_last, bids_, asks_, callback);
}

void Gateway::operator()(Trace<TradeSummary> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Rest::SymbolsUpdate &symbols_update) {
  /*
  auto [size, start_from] = shared_.symbols(symbols_update.symbols);
  ensure_symbol_slices(size);
  for (auto &item : market_data_) {
    (*item).subscribe(start_from);
  }
  for (auto &item : drop_copy_) {
    (*item.second)(symbols_update);
  }
  */
}

void Gateway::ensure_symbol_slices(size_t size) {
  /*
  while (std::size(market_data_) < size) {
    log::info("Create market-data (user-stream)"sv);
    auto market_data = std::make_unique<MarketData>(*this, context_, ++stream_id_, shared_, std::size(market_data_));
    MessageInfo message_info;
    Start start;
    create_event_and_dispatch(*market_data, message_info, start);
    market_data_.emplace_back(std::move(market_data));
  }
  */
}

uint16_t Gateway::operator()(Event<CreateOrder> const &, server::oms::Order const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &,
    server::oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &,
    server::oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(Event<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(Event<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Gateway::operator()(metrics::Writer &writer) const {
  rest_(writer);
  market_data_(writer);
}

template <typename... Args>
void Gateway::dispatch(Args &&...args) {
  auto helper = [&](auto &target) { target(std::forward<Args>(args)...); };
  helper(rest_);
  helper(market_data_);
}

}  // namespace hyperliquid
}  // namespace roq
