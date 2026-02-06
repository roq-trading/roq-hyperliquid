/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::OrderUpdates;

TEST_CASE("create", "[json_order_updates]") {
  auto message = R"({)"
                 R"("channel":"orderUpdates",)"
                 R"("data":[{)"
                 R"("order":{)"
                 R"("coin":"ETH",)"
                 R"("side":"B",)"
                 R"("limitPx":"1500.0",)"
                 R"("sz":"1.0",)"
                 R"("oid":313890189710,)"
                 R"("timestamp":1770381849546,)"
                 R"("origSz":"1.0",)"
                 R"("cloid":"0x00030050c6301a2b00000100000000d2")"
                 R"(},)"
                 R"("status":"open",)"
                 R"("statusTimestamp":1770381849546)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == json::Channel::ORDER_UPDATES);
    REQUIRE(std::size(obj.data) == 1);
    auto &d0 = obj.data[0];
    CHECK(d0.order.coin == "ETH"sv);
    CHECK(d0.order.side == json::Side::BID);
    CHECK(d0.order.limit_px == 1500.0_a);
    CHECK(d0.order.sz == 1.0_a);
    CHECK(d0.order.oid == 313890189710);
    CHECK(d0.order.timestamp == 1770381849546ms);
    CHECK(d0.order.orig_sz == 1.0_a);
    CHECK(d0.order.cloid == "0x00030050c6301a2b00000100000000d2"sv);
    CHECK(d0.status == json::OrderStatus::OPEN);
    CHECK(d0.status_timestamp == 1770381849546ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}

TEST_CASE("cancel", "[json_order_updates]") {
  auto message = R"({)"
                 R"("channel":"orderUpdates",)"
                 R"("data":[{)"
                 R"("order":{)"
                 R"("coin":"ETH",)"
                 R"("side":"B",)"
                 R"("limitPx":"1500.0",)"
                 R"("sz":"1.0",)"
                 R"("oid":313890189710,)"
                 R"("timestamp":1770381849546,)"
                 R"("origSz":"1.0",)"
                 R"("cloid":"0x00030050c6301a2b00000100000000d2")"
                 R"(},)"
                 R"("status":"canceled",)"
                 R"("statusTimestamp":1770381858690)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == json::Channel::ORDER_UPDATES);
    REQUIRE(std::size(obj.data) == 1);
    auto &d0 = obj.data[0];
    CHECK(d0.order.coin == "ETH"sv);
    CHECK(d0.order.side == json::Side::BID);
    CHECK(d0.order.limit_px == 1500.0_a);
    CHECK(d0.order.sz == 1.0_a);
    CHECK(d0.order.oid == 313890189710);
    CHECK(d0.order.timestamp == 1770381849546ms);  // create time
    CHECK(d0.order.orig_sz == 1.0_a);
    CHECK(d0.order.cloid == "0x00030050c6301a2b00000100000000d2"sv);
    CHECK(d0.status == json::OrderStatus::CANCELED);
    CHECK(d0.status_timestamp == 1770381858690ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
