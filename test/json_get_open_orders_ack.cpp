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
                 R"("limitPx":"1500.0",)"
                 R"("sz":"1.0",)"
                 R"("oid":313932628709,)"
                 R"("timestamp":1770385175044,)"
                 R"("origSz":"1.0",)"
                 R"("cloid":"0x00030050c829f0770000010000000073")"
                 R"(},{)"
                 R"("coin":"ETH",)"
                 R"("side":"A",)"
                 R"("limitPx":"2500.0",)"
                 R"("sz":"1.0",)"
                 R"("oid":313932486380,)"
                 R"("timestamp":1770385167498,)"
                 R"("origSz":"1.0",)"
                 R"("cloid":"0x00030050c829f0760000010000000072")"
                 R"(})"
                 R"(])";
  auto helper = [&](value_type &obj) {
    REQUIRE(std::size(obj.data) == 2);
    //
    auto &o0 = obj.data[0];
    CHECK(o0.coin == "ETH"sv);
    CHECK(o0.side == json::Side::BID);
    CHECK(o0.limit_px == 1500.0_a);
    CHECK(o0.sz == 1.0_a);
    CHECK(o0.oid == 313932628709);
    CHECK(o0.timestamp == 1770385175044ms);
    CHECK(o0.orig_sz == 1.0_a);
    CHECK(o0.cloid == "0x00030050c829f0770000010000000073"sv);
    //
    auto &o1 = obj.data[1];
    CHECK(o1.coin == "ETH"sv);
    CHECK(o1.side == json::Side::ASK);
    CHECK(o1.limit_px == 2500.0_a);
    CHECK(o1.sz == 1.0_a);
    CHECK(o1.oid == 313932486380);
    CHECK(o1.timestamp == 1770385167498ms);
    CHECK(o1.orig_sz == 1.0_a);
    CHECK(o1.cloid == "0x00030050c829f0760000010000000072"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
