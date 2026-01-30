/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/get_spot_clearing_house_state_ack.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::GetSpotClearingHouseStateAck;

TEST_CASE("empty", "[json_get_spot_clearing_house_state_ack]") {
  auto message = R"({)"
                 R"("balances":[])"
                 R"(})";
  auto helper = [&](value_type &obj) { REQUIRE(std::empty(obj.balances)); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
