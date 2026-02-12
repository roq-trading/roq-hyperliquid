/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/hyperliquid/tools/conversions.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

namespace {
std::string price_helper(auto price, auto decimals, auto is_spot) {
  auto tmp = tools::Conversions::roundPrice(price, decimals, is_spot);
  return tools::Conversions::floatToWire(tmp);
}

std::string quantity_helper(auto price, auto decimals) {
  auto tmp = tools::Conversions::roundSize(price, decimals);
  return tools::Conversions::floatToWire(tmp);
}
}  // namespace

TEST_CASE("format_price", "[tools_conversions]") {
  CHECK(price_helper(123.4, 2, false) == "123.4"s);
  CHECK(price_helper(123.45, 2, false) == "123.45"s);
  CHECK(price_helper(123.455, 2, false) == "123.46"s);
}

TEST_CASE("format_quantity", "[tools_conversions]") {
  CHECK(quantity_helper(123.4, 2) == "123.4"s);
  CHECK(quantity_helper(123.45, 2) == "123.45"s);
  CHECK(quantity_helper(123.455, 2) == "123.46"s);
}
