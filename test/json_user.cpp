/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::User;

TEST_CASE("funding", "[json_user]") {
  auto message = R"({)"
                 R"("channel":"user",)"
                 R"("data":{)"
                 R"("funding":{)"
                 R"("time":1770523200069,)"
                 R"("coin":"BTC",)"
                 R"("usdc":"0.000021",)"
                 R"("szi":"0.00004",)"
                 R"("fundingRate":"-0.0000079506",)"
                 R"("nSamples":null)"
                 R"(})"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == json::Channel::USER);
    CHECK(obj.data.funding.time == 1770523200069ms);
    CHECK(obj.data.funding.coin == "BTC"sv);
    CHECK(obj.data.funding.usdc == 0.000021_a);
    CHECK(obj.data.funding.szi == 0.00004_a);
    CHECK(obj.data.funding.funding_rate == -0.0000079506_a);
    // n_samples
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}

TEST_CASE("fills", "[json_user]") {
  auto message = R"({)"
                 R"("channel":"user",)"
                 R"("data":{)"
                 R"("fills":[{)"
                 R"("coin":"ETH",)"
                 R"("px":"2067.2",)"
                 R"("sz":"0.01",)"
                 R"("side":"A",)"
                 R"("time":1770524192528,)"
                 R"("startPosition":"0.01",)"
                 R"("dir":"Close Long",)"
                 R"("closedPnl":"-0.13",)"
                 R"("hash":"0x5b1955fc530a1a7e5c930434e47ff202074e00e1ee0d3950fee2014f120df468",)"
                 R"("oid":315449302232,)"
                 R"("crossed":true,)"
                 R"("fee":"0.00893",)"
                 R"("tid":343484124188045,)"
                 R"("cloid":"0x000300511afb52d40000010000000073",)"
                 R"("feeToken":"USDC",)"
                 R"("twapId":null)"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == json::Channel::USER);
    REQUIRE(std::size(obj.data.fills) == 1);
    /*
    CHECK(obj.data.funding.time == 1770523200069ms);
    CHECK(obj.data.funding.coin == "BTC"sv);
    CHECK(obj.data.funding.usdc == 0.000021_a);
    CHECK(obj.data.funding.szi == 0.00004_a);
    CHECK(obj.data.funding.funding_rate == -0.0000079506_a);
    // n_samples
    */
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
