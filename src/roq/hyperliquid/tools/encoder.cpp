/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/encoder.hpp"

#include "roq/logging.hpp"

#include "roq/utils/common.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/hyperliquid/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
bool is_buy_helper(auto side) {
  switch (side) {
    using enum Side;
    case UNDEFINED:
      break;
    case BUY:
      return true;
    case SELL:
      return false;
  }
  log::fatal("Unexpected"sv);
}

auto order_type_helper(auto &order) -> crypto::OrderType {
  auto limit_order_type_helper = [&]() -> crypto::LimitOrderType {
    switch (order.order_type) {
      using enum OrderType;
      case UNDEFINED:
        break;
      case MARKET:
        break;
      case LIMIT: {
        auto tif = map(order.time_in_force).template get<json::TimeInForce>();
        return {
            .tif = std::string{tif.as_raw_text()},
        };
      }
    }
    log::fatal("Unexpected"sv);
  };
  return {
      .limit = limit_order_type_helper(),
      .trigger = {},
  };
}
}  // namespace

// === IMPLEMENTATION ===

std::pair<std::string, std::vector<uint8_t>> Encoder::create_order(
    CreateOrder const &create_order,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::chrono::milliseconds now_utc,
    crypto::Exchange &exchange) {
  std::string coin{order.symbol};
  auto is_buy = is_buy_helper(order.side);
  auto reduce_only = false;  // XXX FIXME TODO
  auto order_type = order_type_helper(order);
  auto tmp = fmt::format("0x{:0>32}"sv, request_id);
  crypto::Cloid cloid{tmp};
  auto action = exchange.ROQ_order(
      coin,
      order.external_security_id,  // note!
      utils::decimal_digits(order.quantity_precision.precision),
      utils::decimal_digits(order.price_precision.precision),
      is_buy,
      create_order.quantity,
      create_order.price,
      order_type,
      reduce_only,
      cloid);
  auto hash = exchange.ROQ_actionHash(action, now_utc);
  auto action_2 = action.dump();
  return {action_2, hash};
}

std::pair<std::string, std::vector<uint8_t>> Encoder::modify_order(
    ModifyOrder const &modify_order,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds now_utc,
    crypto::Exchange &exchange) {
  std::string coin{order.symbol};
  auto tmp = fmt::format("0x{:0>32}"sv, static_cast<std::string_view>(order.client_order_id));
  crypto::Cloid cloid{tmp};
  crypto::OidOrCloid oid{cloid};
  auto is_buy = is_buy_helper(order.side);
  auto reduce_only = true;  // XXX FIXME TODO
  auto order_type = order_type_helper(order);
  auto action =
      exchange.ROQ_modifyOrder(oid, coin, order.external_security_id, is_buy, modify_order.quantity, modify_order.price, order_type, reduce_only, cloid);
  auto hash = exchange.ROQ_actionHash(action, now_utc);
  auto action_2 = action.dump();
  return {action_2, hash};
}

std::pair<std::string, std::vector<uint8_t>> Encoder::cancel_order(
    CancelOrder const &,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds now_utc,
    crypto::Exchange &exchange) {
  std::string coin{order.symbol};
  if (std::empty(order.external_order_id)) {
    auto tmp = fmt::format("0x{:0>32}"sv, static_cast<std::string_view>(order.client_order_id));
    crypto::Cloid cloid{tmp};
    auto action = exchange.ROQ_cancelByCloid(coin, order.external_security_id, cloid);
    auto hash = exchange.ROQ_actionHash(action, now_utc);
    auto action_2 = action.dump();
    return {action_2, hash};
  } else {
    auto oid = utils::charconv::from_chars<int64_t>(order.external_order_id);
    log::warn("DEBUG oid={}, asset={}"sv, oid, order.external_security_id);
    auto action = exchange.ROQ_cancel(coin, order.external_security_id, oid);
    auto hash = exchange.ROQ_actionHash(action, now_utc);
    auto action_2 = action.dump();
    return {action_2, hash};
  }
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
