/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <span>

#include "roq/tick_size_step.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct TickSizeSteps final {
  static std::span<TickSizeStep const> get(int sz_decimals, bool is_spot);
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
