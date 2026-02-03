/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/get_clearing_house_state_ack.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::GetClearingHouseStateAck;

TEST_CASE("empty", "[json_get_clearing_house_state_ack]") {
  auto message = R"({)"
                 R"("marginSummary":{)"
                 R"("accountValue":"0.0",)"
                 R"("totalNtlPos":"0.0",)"
                 R"("totalRawUsd":"0.0",)"
                 R"("totalMarginUsed":"0.0")"
                 R"(},)"
                 R"("crossMarginSummary":{)"
                 R"("accountValue":"0.0",)"
                 R"("totalNtlPos":"0.0",)"
                 R"("totalRawUsd":"0.0",)"
                 R"("totalMarginUsed":"0.0")"
                 R"(},)"
                 R"("crossMaintenanceMarginUsed":"0.0",)"
                 R"("withdrawable":"0.0",)"
                 R"("assetPositions":[],)"
                 R"("time":1769754306819)"
                 R"(})";
  auto helper = [&](value_type &obj) {
    //
    CHECK(obj.cross_maintenance_margin_used == 0.0_a);
    CHECK(obj.withdrawable == 0.0_a);
    CHECK(obj.time == 1769754306819ms);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("simple", "[json_get_clearing_house_state_ack]") {
  auto message = R"({)"
                 R"("marginSummary":{)"
                 R"("accountValue":"179.007426",)"
                 R"("totalNtlPos":"47.66235",)"
                 R"("totalRawUsd":"131.345076",)"
                 R"("totalMarginUsed":"2.383117")"
                 R"(},)"
                 R"("crossMarginSummary":{)"
                 R"("accountValue":"179.007426",)"
                 R"("totalNtlPos":"47.66235",)"
                 R"("totalRawUsd":"131.345076",)"
                 R"("totalMarginUsed":"2.383117")"
                 R"(},)"
                 R"("crossMaintenanceMarginUsed":"0.595779",)"
                 R"("withdrawable":"174.241191",)"
                 R"("assetPositions":[{)"
                 R"("type":"oneWay",)"
                 R"("position":{)"
                 R"("coin":"BTC",)"
                 R"("szi":"0.00061",)"
                 R"("leverage":{)"
                 R"("type":"cross",)"
                 R"("value":20)"
                 R"(},)"
                 R"("entryPx":"82771.8",)"
                 R"("positionValue":"47.66235",)"
                 R"("unrealizedPnl":"-2.82848",)"
                 R"("returnOnEquity":"-1.1203935447",)"
                 R"("liquidationPx":null,)"
                 R"("marginUsed":"2.383117",)"
                 R"("maxLeverage":40,)"
                 R"("cumFunding":{)"
                 R"("allTime":"0.004156",)"
                 R"("sinceOpen":"0.004156",)"
                 R"("sinceChange":"-0.000221")"
                 R"(})"
                 R"(})"
                 R"(})"
                 R"(],)"
                 R"("time":1770118781558)"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.margin_summary.account_value == 179.007426_a);
    CHECK(obj.margin_summary.total_ntl_pos == 47.66235_a);
    CHECK(obj.margin_summary.total_raw_usd == 131.345076_a);
    CHECK(obj.margin_summary.total_margin_used == 2.383117_a);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
