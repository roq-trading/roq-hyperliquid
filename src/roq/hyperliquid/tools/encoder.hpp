/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#include "roq/cancel_order.hpp"
#include "roq/create_order.hpp"
#include "roq/modify_order.hpp"

#include "roq/server/oms/order.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct Encoder final {
  static std::pair<std::string, std::vector<uint8_t>> create_order(
      CreateOrder const &, server::oms::Order const &, std::string_view const &request_id, std::chrono::milliseconds now_utc);

  static std::pair<std::string, std::vector<uint8_t>> modify_order(
      ModifyOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds now_utc);

  static std::pair<std::string, std::vector<uint8_t>> cancel_order(
      CancelOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds now_utc);
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
