/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/get_clearing_house_state_ack.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::GetClearingHouseStateAck;

TEST_CASE("simple", "[json_get_clearing_house_state_ack]") {
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
