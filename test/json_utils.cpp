/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/hyperliquid/crypto/conversions.hpp"
#include "roq/hyperliquid/crypto/signing.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

namespace {
std::string price_helper(auto price, auto decimals, auto is_spot) {
  auto tmp = crypto::roundPrice(price, decimals, is_spot);
  return crypto::floatToWire(tmp);
}

std::string quantity_helper(auto price, auto decimals) {
  auto tmp = crypto::roundSize(price, decimals);
  return crypto::floatToWire(tmp);
}
}  // namespace

TEST_CASE("format_price", "[json_utils]") {
  CHECK(price_helper(123.4, 2, false) == "123.4"s);
  CHECK(price_helper(123.45, 2, false) == "123.45"s);
  CHECK(price_helper(123.455, 2, false) == "123.46"s);
}

TEST_CASE("format_quantity", "[json_utils]") {
  CHECK(quantity_helper(123.4, 2) == "123.4"s);
  CHECK(quantity_helper(123.45, 2) == "123.45"s);
  CHECK(quantity_helper(123.455, 2) == "123.46"s);
}
