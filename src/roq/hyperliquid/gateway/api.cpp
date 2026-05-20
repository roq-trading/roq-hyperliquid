/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/gateway/api.hpp"

#include "roq/exceptions.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace gateway {

// === IMPLEMENTATION ===

API API::create(Settings const &) {
  return {
      .market_data{
          .get_info = "/info"sv,
      },
      .simple{
          .account_info = "/v5/account/info"sv,
          .account_wallet_balance = "/v5/account/wallet-balance"sv,
          .position_list = "/v5/position/list"sv,
          .order_realtime = "/v5/order/realtime"sv,
          .execution_list = "/v5/execution/list"sv,
          .order_create = "/exchange"sv,
          .order_amend = "/v5/order/amend"sv,
          .order_cancel = "/v5/order/cancel"sv,
          .order_cancel_all = "/v5/order/cancel-all"sv,
      },
  };
}

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
