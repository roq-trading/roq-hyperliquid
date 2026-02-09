/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

namespace roq {
namespace hyperliquid {
namespace tools {

struct Conversions final {
  static double roundPrice(double price, int sz_decimals, bool is_spot);

  static double roundSize(double size, int sz_decimals);

  static std::string floatToWire(double value);
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
