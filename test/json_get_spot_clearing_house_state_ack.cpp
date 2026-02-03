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

TEST_CASE("simple", "[json_get_spot_clearing_house_state_ack]") {
  auto message = R"({)"
                 R"("balances":[{)"
                 R"("coin":"USDC",)"
                 R"("token":0,)"
                 R"("total":"0.09515412",)"
                 R"("hold":"0.0",)"
                 R"("entryNtl":"0.0")"
                 R"(},{)"
                 R"("coin":"USDH",)"
                 R"("token":360,)"
                 R"("total":"0.0",)"
                 R"("hold":"0.0",)"
                 R"("entryNtl":"0.0")"
                 R"(})"
                 R"(])"
                 R"(})";
  auto helper = [&](value_type &obj) {
    REQUIRE(std::size(obj.balances) == 2);
    auto &b0 = obj.balances[0];
    CHECK(b0.coin == "USDC"sv);
    CHECK(b0.token == 0);
    CHECK(b0.total == 0.09515412_a);
    CHECK(b0.hold == 0.0_a);
    CHECK(b0.entry_ntl == 0.0_a);
    auto &b1 = obj.balances[1];
    CHECK(b1.coin == "USDH"sv);
    CHECK(b1.token == 360);
    CHECK(b1.total == 0.0_a);
    CHECK(b1.hold == 0.0_a);
    CHECK(b1.entry_ntl == 0.0_a);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
