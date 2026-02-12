/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/hyperliquid/tools/key.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

TEST_CASE("simple", "[tools_key]") {
  auto key = tools::Key::create_from_private_key("0x0123456789012345678901234567890123456789012345678901234567890123"sv);
  CHECK(key.derive_address() == "0x14791697260e4c9a71f18484c9f997b308e59325"sv);
}
