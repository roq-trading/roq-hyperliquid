/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/get_open_orders_ack.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::GetOpenOrdersAck;

TEST_CASE("simple", "[json_get_open_orders_ack]") {
  auto message = R"([{)"
                 R"("coin":"ETH",)"
                 R"("side":"B",)"
                 R"("limitPx":"1800.0",)"
                 R"("sz":"1.0",)"
                 R"("oid":311642020084,)"
                 R"("timestamp":1770219647089,)"
                 R"("origSz":"1.0",)"
                 R"("cloid":"0x430882655000030000000001000000b9")"
                 R"(})"
                 R"(])";
  auto helper = [&](value_type &obj) {
    REQUIRE(std::size(obj.data) == 1);
    auto &o0 = obj.data[0];
    CHECK(o0.coin == "ETH"sv);
    CHECK(o0.side == "B"sv);
    CHECK(o0.limit_px == 1800.0_a);
    CHECK(o0.sz == 1.0_a);
    CHECK(o0.oid == 311642020084);
    CHECK(o0.timestamp == 1770219647089ms);
    CHECK(o0.orig_sz == 1.0_a);
    CHECK(o0.cloid == "0x430882655000030000000001000000b9"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
