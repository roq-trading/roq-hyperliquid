/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/hyperliquid/settings.hpp"

#include "roq/hyperliquid/tools/api.hpp"

#include "roq/hyperliquid/json/category.hpp"

namespace roq {
namespace hyperliquid {

struct API final {
  struct {
    std::string_view market_instrument_info;
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

  tools::API api;
  json::Category category;

  // factory
  static API create(Settings const &);
};

}  // namespace hyperliquid
}  // namespace roq
