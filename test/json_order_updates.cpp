/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::OrderUpdates;

TEST_CASE("create_resting", "[json_order_updates]") {
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

TEST_CASE("create_failure_min_notional", "[json_order_updates]") {
  auto message = R"({)"
                 R"("channel":"orderUpdates",)"
                 R"("data":[{)"
                 R"("order":{)"
                 R"("coin":"ETH",)"
                 R"("side":"B",)"
                 R"("limitPx":"1500.0",)"
                 R"("sz":"0.0001",)"
                 R"("oid":315435025155,)"
                 R"("timestamp":1770522010049,)"
                 R"("origSz":"0.0001",)"
                 R"("cloid":"0x0003005119ba40ba000001000000004d")"
                 R"(},)"
                 R"("status":"minTradeNtlRejected",)"
                 R"("statusTimestamp":1770522010049)"
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
    CHECK(d0.order.sz == 0.0001_a);
    CHECK(d0.order.oid == 315435025155);
    CHECK(d0.order.timestamp == 1770522010049ms);
    CHECK(d0.order.orig_sz == 0.0001_a);
    CHECK(d0.order.cloid == "0x0003005119ba40ba000001000000004d"sv);
    CHECK(d0.status == json::OrderStatus::MIN_TRADE_NTL_REJECTED);
    CHECK(d0.status_timestamp == 1770522010049ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}

TEST_CASE("create_failure_", "[json_order_updates]") {
  auto message = R"({)"
                 R"("channel":"orderUpdates",)"
                 R"("data":[{)"
                 R"("order":{)"
                 R"("coin":"SOL",)"
                 R"("side":"B",)"
                 R"("limitPx":"50.0",)"
                 R"("sz":"1000.0",)"
                 R"("oid":321519065621,)"
                 R"("timestamp":1771168943567,)"
                 R"("origSz":"1000.0",)"
                 R"("cloid":"0x000300529b522d0e00000100000000fd")"
                 R"(},)"
                 R"("status":"perpMarginRejected",)"
                 R"("statusTimestamp":1771168943567)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == json::Channel::ORDER_UPDATES);
    REQUIRE(std::size(obj.data) == 1);
    auto &d0 = obj.data[0];
    CHECK(d0.order.coin == "SOL"sv);
    CHECK(d0.order.side == json::Side::BID);
    CHECK(d0.order.limit_px == 50.0_a);
    CHECK(d0.order.sz == 1000.0_a);
    CHECK(d0.order.oid == 321519065621);
    CHECK(d0.order.timestamp == 1771168943567ms);
    CHECK(d0.order.orig_sz == 1000.0_a);
    CHECK(d0.order.cloid == "0x000300529b522d0e00000100000000fd"sv);
    CHECK(d0.status == json::OrderStatus::PERP_MARGIN_REJECTED);
    CHECK(d0.status_timestamp == 1771168943567ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}

TEST_CASE("create_filled", "[json_order_updates]") {
  auto message = R"({)"
                 R"("channel":"orderUpdates",)"
                 R"("data":[{)"
                 R"("order":{)"
                 R"("coin":"ETH",)"
                 R"("side":"B",)"
                 R"("limitPx":"2080.0",)"
                 R"("sz":"0.01",)"
                 R"("oid":315452584853,)"
                 R"("timestamp":1770524603232,)"
                 R"("origSz":"0.01",)"
                 R"("cloid":"0x000300511b45504e0000010000000054")"
                 R"(},)"
                 R"("status":"open",)"
                 R"("statusTimestamp":1770524603232)"
                 R"(},{)"
                 R"("order":{)"
                 R"("coin":"ETH",)"
                 R"("side":"B",)"
                 R"("limitPx":"2080.0",)"
                 R"("sz":"0.0",)"
                 R"("oid":315452584853,)"
                 R"("timestamp":1770524603232,)"
                 R"("origSz":"0.01",)"
                 R"("cloid":"0x000300511b45504e0000010000000054")"
                 R"(},)"
                 R"("status":"filled",)"
                 R"("statusTimestamp":1770524603232)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == json::Channel::ORDER_UPDATES);
    REQUIRE(std::size(obj.data) == 2);
    //
    auto &d0 = obj.data[0];
    CHECK(d0.order.coin == "ETH"sv);
    CHECK(d0.order.side == json::Side::BID);
    CHECK(d0.order.limit_px == 2080.0_a);
    CHECK(d0.order.sz == 0.01_a);
    CHECK(d0.order.oid == 315452584853);
    CHECK(d0.order.timestamp == 1770524603232ms);
    CHECK(d0.order.orig_sz == 0.01_a);
    CHECK(d0.order.cloid == "0x000300511b45504e0000010000000054"sv);
    CHECK(d0.status == json::OrderStatus::OPEN);
    CHECK(d0.status_timestamp == 1770524603232ms);
    //
    auto &d1 = obj.data[1];
    CHECK(d1.order.coin == "ETH"sv);
    CHECK(d1.order.side == json::Side::BID);
    CHECK(d1.order.limit_px == 2080.0_a);
    CHECK(d1.order.sz == 0.0_a);  // note!
    CHECK(d1.order.oid == 315452584853);
    CHECK(d1.order.timestamp == 1770524603232ms);
    CHECK(d1.order.orig_sz == 0.01_a);
    CHECK(d1.order.cloid == "0x000300511b45504e0000010000000054"sv);
    CHECK(d1.status == json::OrderStatus::FILLED);
    CHECK(d1.status_timestamp == 1770524603232ms);
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
