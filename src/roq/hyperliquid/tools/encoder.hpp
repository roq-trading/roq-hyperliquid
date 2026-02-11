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
  static std::pair<std::string, std::vector<std::byte>> create_order(
      CreateOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::chrono::milliseconds now_utc,
      std::chrono::milliseconds expires_after_utc);

  static std::pair<std::string, std::vector<std::byte>> modify_order(
      ModifyOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds now_utc,
      std::chrono::milliseconds expires_after_utc);

  static std::pair<std::string, std::vector<std::byte>> cancel_order(
      CancelOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds now_utc,
      std::chrono::milliseconds expires_after_utc);
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
