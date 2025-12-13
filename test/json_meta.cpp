/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/meta.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

namespace {
// selective
auto const MESSAGE = R"({)"
                     R"("universe":[)"
                     R"({"szDecimals":5,"name":"BTC","maxLeverage":40,"marginTableId":56},)"
                     R"({"szDecimals":4,"name":"ETH","maxLeverage":25,"marginTableId":55},)"
                     R"({"szDecimals":1,"name":"MATIC","maxLeverage":20,"marginTableId":20,"isDelisted":true},)"
                     R"({"szDecimals":0,"name":"LINEA","maxLeverage":3,"marginTableId":3,"onlyIsolated":true})"
                     R"(],)"
                     R"("marginTables":[)"
                     R"([50,{"description":"","marginTiers":[)"
                     R"({"lowerBound":"0.0","maxLeverage":50})"
                     R"(])"
                     R"(})"
                     R"(],)"
                     R"([51,{"description":"tiered 10x","marginTiers":[)"
                     R"({"lowerBound":"0.0","maxLeverage":10},)"
                     R"({"lowerBound":"3000000.0","maxLeverage":5})"
                     R"(])"
                     R"(})"
                     R"(],)"
                     R"([52,{"description":"tiered 10x","marginTiers":[)"
                     R"({"lowerBound":"0.0","maxLeverage":10},)"
                     R"({"lowerBound":"20000000.0","maxLeverage":5})"
                     R"(])"
                     R"(})"
                     R"(],)"
                     R"([53,{"description":"tiered 20x","marginTiers":[)"
                     R"({"lowerBound":"0.0","maxLeverage":20},)"
                     R"({"lowerBound":"40000000.0","maxLeverage":10})"
                     R"(])"
                     R"(})"
                     R"(],)"
                     R"([54,{"description":"tiered 20x","marginTiers":[)"
                     R"({"lowerBound":"0.0","maxLeverage":20},)"
                     R"({"lowerBound":"70000000.0","maxLeverage":10})"
                     R"(])"
                     R"(})"
                     R"(],)"
                     R"([55,{"description":"tiered 25x","marginTiers":[)"
                     R"({"lowerBound":"0.0","maxLeverage":25},)"
                     R"({"lowerBound":"100000000.0","maxLeverage":15})"
                     R"(])"
                     R"(})"
                     R"(],)"
                     R"([56,{"description":"tiered 40x","marginTiers":[)"
                     R"({"lowerBound":"0.0","maxLeverage":40},)"
                     R"({"lowerBound":"150000000.0","maxLeverage":20})"
                     R"(])"
                     R"(})"
                     R"(])"
                     R"(])"
                     R"(})";

}  // namespace

TEST_CASE("json_meta", "[json_meta]") {
  core::json::BufferStack buffer{8192, 1};
  json::Meta obj{MESSAGE, buffer};
  auto &u = obj.universe;
  REQUIRE(std::size(u) == 4);
  CHECK(u[0].sz_decimals == 5);
  CHECK(u[3].sz_decimals == 0);
  // XXX FIXME TODO can't parse key-value array -- need something new
  // auto &mt = obj.margin_tables;
  // REQUIRE(std::size(margin_tables) == 7);
}
