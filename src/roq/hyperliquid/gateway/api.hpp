/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/hyperliquid/gateway/settings.hpp"

namespace roq {
namespace hyperliquid {
namespace gateway {

struct API final {
  struct {
    std::string_view get_info;
  } market_data;

  struct {
    std::string_view account_info;
    std::string_view account_wallet_balance;
    std::string_view position_list;
    std::string_view order_realtime;
    std::string_view execution_list;
    std::string_view order_create;
    std::string_view order_amend;
    std::string_view order_cancel;
    std::string_view order_cancel_all;
  } simple;

  // factory
  static API create(Settings const &);
};

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
