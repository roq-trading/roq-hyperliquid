/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/tick_size_steps.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === CONSTANTS ===

namespace {
// - spot
auto const SPOT_0 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const SPOT_1 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const SPOT_2 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const SPOT_3 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const SPOT_4 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const SPOT_5 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const SPOT_6 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const SPOT_7 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

// - perps

auto const PERPS_0 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const PERPS_1 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const PERPS_2 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const PERPS_3 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const PERPS_4 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};

auto const PERPS_5 = std::array<TickSizeStep, 6>{{
    {.min_price = -std::numeric_limits<double>::infinity(), .tick_size = 0.0001},
    {.min_price = 10.0, .tick_size = 0.001},
    {.min_price = 100.0, .tick_size = 0.01},
    {.min_price = 1000.0, .tick_size = 0.1},
    {.min_price = 10000.0, .tick_size = 1.0},
    {.min_price = 100000.0, .tick_size = 10.0},
}};
}  // namespace

// === IMPLEMENTATION ===

std::span<TickSizeStep const> TickSizeSteps::get(int sz_decimals, bool is_spot) {
  // prices can have up to 5 significant figures, but no more than MAX_DECIMALS - szDecimals decimal places
  // integer prices are always allowed, regardless of the number of significant figures
  if (is_spot) {
    // MAX_DECIMALS is 8 for spot
    switch (sz_decimals) {
      case 0:
        return SPOT_0;
      case 1:
        return SPOT_1;
      case 2:
        return SPOT_2;
      case 3:
        return SPOT_3;
      case 4:
        return SPOT_4;
      case 5:
        return SPOT_5;
      case 6:
        return SPOT_6;
      case 7:
        return SPOT_7;
    }
    log::fatal("Unexpected: sz_decimals={}"sv, sz_decimals);
  } else {
    // MAX_DECIMALS is 6 for perps
    switch (sz_decimals) {
      case 0:
        return PERPS_0;
      case 1:
        return PERPS_1;
      case 2:
        return PERPS_2;
      case 3:
        return PERPS_3;
      case 4:
        return PERPS_4;
      case 5:
        return PERPS_5;
    }
    log::fatal("Unexpected: sz_decimals={}"sv, sz_decimals);
  }
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
