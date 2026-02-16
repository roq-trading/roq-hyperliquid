/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include <array>

#include "roq/tick_size_step.hpp"

#include "roq/hyperliquid/tools/conversions.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

namespace {
auto const EPS = 1.0e-8;

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
  CHECK(price_helper(1.0, 2, false) == "1"s);
  CHECK(price_helper(1.5, 2, false) == "1.5"s);
  CHECK(price_helper(1.05, 2, false) == "1.05"s);
  CHECK(price_helper(1.005, 2, false) == "1.005"s);
  CHECK(price_helper(1.0005, 2, false) == "1.0005"s);
  CHECK(price_helper(1.00005, 2, false) == "1.0001"s);
  CHECK(price_helper(1.000005, 2, false) == "1"s);

  CHECK(price_helper(9.0, 2, false) == "9"s);
  CHECK(price_helper(9.5, 2, false) == "9.5"s);
  CHECK(price_helper(9.05, 2, false) == "9.05"s);
  CHECK(price_helper(9.005, 2, false) == "9.005"s);
  CHECK(price_helper(9.0005, 2, false) == "9.0005"s);
  CHECK(price_helper(9.00005, 2, false) == "9.0001"s);
  CHECK(price_helper(9.000005, 2, false) == "9"s);

  CHECK(price_helper(10.0, 2, false) == "10"s);
  CHECK(price_helper(10.5, 2, false) == "10.5"s);
  CHECK(price_helper(10.05, 2, false) == "10.05"s);
  CHECK(price_helper(10.005, 2, false) == "10.005"s);
  CHECK(price_helper(10.0005, 2, false) == "10.001"s);
  CHECK(price_helper(10.00005, 2, false) == "10"s);

  CHECK(price_helper(99.0, 2, false) == "99"s);
  CHECK(price_helper(99.5, 2, false) == "99.5"s);
  CHECK(price_helper(99.05, 2, false) == "99.05"s);
  CHECK(price_helper(99.005, 2, false) == "99.005"s);
  CHECK(price_helper(99.0005, 2, false) == "99.001"s);
  CHECK(price_helper(99.00005, 2, false) == "99"s);

  CHECK(price_helper(100.0, 2, false) == "100"s);
  CHECK(price_helper(100.5, 2, false) == "100.5"s);
  CHECK(price_helper(100.05, 2, false) == "100.05"s);
  CHECK(price_helper(100.005, 2, false) == "100.01"s);
  CHECK(price_helper(100.0005, 2, false) == "100"s);

  CHECK(price_helper(999.0, 2, false) == "999"s);
  CHECK(price_helper(999.5, 2, false) == "999.5"s);
  CHECK(price_helper(999.05, 2, false) == "999.05"s);
  CHECK(price_helper(999.005, 2, false) == "999.01"s);
  CHECK(price_helper(999.0005, 2, false) == "999"s);

  CHECK(price_helper(1000.0, 2, false) == "1000"s);
  CHECK(price_helper(1000.5, 2, false) == "1000.5"s);
  CHECK(price_helper(1000.05 + EPS, 2, false) == "1000.1"s);
  CHECK(price_helper(1000.005, 2, false) == "1000"s);

  CHECK(price_helper(9999.0, 2, false) == "9999"s);
  CHECK(price_helper(9999.5, 2, false) == "9999.5"s);
  CHECK(price_helper(9999.05 + EPS, 2, false) == "9999.1"s);
  CHECK(price_helper(9999.005, 2, false) == "9999"s);

  CHECK(price_helper(10000.0, 2, false) == "10000"s);
  CHECK(price_helper(10000.5, 2, false) == "10001"s);

  CHECK(price_helper(99999.0, 2, false) == "99999"s);
  CHECK(price_helper(99999.5, 2, false) == "100000"s);

  CHECK(price_helper(100000.0, 2, false) == "100000"s);
  CHECK(price_helper(100005.0 + EPS, 2, false) == "100010"s);  // XXX

  CHECK(price_helper(999999.0, 2, false) == "999999"s);
}

TEST_CASE("format_price_tick_size_steps", "[tools_conversions]") {
  std::array<TickSizeStep, 6> decimals_2{{
      {.min_price = 0.0, .tick_size = 0.0001},
      {.min_price = 10.0, .tick_size = 0.001},
      {.min_price = 100.0, .tick_size = 0.01},
      {.min_price = 1000.0, .tick_size = 0.1},
      {.min_price = 10000.0, .tick_size = 1.0},
      {.min_price = 100000.0, .tick_size = 10.0},
  }};
}

TEST_CASE("format_quantity", "[tools_conversions]") {
  CHECK(quantity_helper(0.0, 2) == "0"s);
  CHECK(quantity_helper(0.5, 2) == "0.5"s);
  CHECK(quantity_helper(0.05, 2) == "0.05"s);
  CHECK(quantity_helper(0.005, 2) == "0.01"s);
  CHECK(quantity_helper(0.0005, 2) == "0"s);

  CHECK(quantity_helper(1.0, 2) == "1"s);
  CHECK(quantity_helper(1.5, 2) == "1.5"s);
  CHECK(quantity_helper(1.05, 2) == "1.05"s);
  CHECK(quantity_helper(1.005 + EPS, 2) == "1.01"s);
  CHECK(quantity_helper(1.0005, 2) == "1"s);

  CHECK(quantity_helper(9.0, 2) == "9"s);
  CHECK(quantity_helper(9.5, 2) == "9.5"s);
  CHECK(quantity_helper(9.05, 2) == "9.05"s);
  CHECK(quantity_helper(9.005, 2) == "9.01"s);
  CHECK(quantity_helper(9.0005, 2) == "9"s);

  CHECK(quantity_helper(999.0, 2) == "999"s);
  CHECK(quantity_helper(999.5, 2) == "999.5"s);
  CHECK(quantity_helper(999.05, 2) == "999.05"s);
  CHECK(quantity_helper(999.005, 2) == "999.01"s);
  CHECK(quantity_helper(999.0005, 2) == "999"s);
}
