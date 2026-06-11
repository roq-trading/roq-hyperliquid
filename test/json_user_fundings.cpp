/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::UserFundings;

// note! trunacted
TEST_CASE("simple", "[json_user_fundings]") {
  auto message = R"({)"
                 R"("channel":"userFundings",)"
                 R"("data":{)"
                 R"("isSnapshot":true,)"
                 R"("user":"0x61fb9e7583dc92cc76eb52f93d88addaa0f02265",)"
                 R"("fundings":[{)"
                 R"("time":1770069600099,)"
                 R"("coin":"BTC",)"
                 R"("usdc":"0.000502",)"
                 R"("szi":"0.00061",)"
                 R"("fundingRate":"-0.0000104821",)"
                 R"("nSamples":null)"
                 R"(},{)"
                 R"("time":1770073200055,)"
                 R"("coin":"BTC",)"
                 R"("usdc":"0.000468",)"
                 R"("szi":"0.00061",)"
                 R"("fundingRate":"-0.0000097486",)"
                 R"("nSamples":null)"
                 R"(},{)"
                 R"("time":1770372000053,)"
                 R"("coin":"BTC",)"
                 R"("usdc":"-0.000001",)"
                 R"("szi":"0.00004",)"
                 R"("fundingRate":"0.0000004105",)"
                 R"("nSamples":null)"
                 R"(},{)"
                 R"("time":1770372000053,)"
                 R"("coin":"xyz:SILVER",)"
                 R"("usdc":"-0.000794",)"
                 R"("szi":"0.26",)"
                 R"("fundingRate":"0.0000412775",)"
                 R"("nSamples":null)"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == protocol::json::Channel::USER_FUNDINGS);
    /*
    auto &data = obj.data;
    CHECK(data.coin == "BTC"sv);
    CHECK(data.time == 1757130041083ms);
    auto &levels = data.levels;
    REQUIRE(std::size(levels) == 2);
    auto &b = levels[0].data;
    REQUIRE(std::size(b) == 20);
    CHECK(b[0].n == 16);
    CHECK(b[19].n == 2);
    auto &a = levels[1].data;
    REQUIRE(std::size(a) == 20);
    CHECK(a[0].n == 21);
    CHECK(a[19].n == 4);
    */
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
