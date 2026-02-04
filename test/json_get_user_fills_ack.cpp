/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/get_user_fills_ack.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::GetUserFillsAck;

TEST_CASE("simple", "[json_get_user_fills_ack]") {
  auto message = R"([{)"
                 R"("coin":"xyz:SILVER",)"
                 R"("px":"80.601",)"
                 R"("sz":"0.6",)"
                 R"("side":"A",)"
                 R"("time":1769986907573,)"
                 R"("startPosition":"0.6",)"
                 R"("dir":"Close Long",)"
                 R"("closedPnl":"-1.2843",)"
                 R"("hash":"0x8e8de2718fa08764900704347f47410204dd00572aa3a63632568dc44ea4614f",)"
                 R"("oid":308800014194,)"
                 R"("crossed":true,)"
                 R"("fee":"0.004178",)"
                 R"("tid":1031003079291799,)"
                 R"("liquidation":{)"
                 R"("liquidatedUser":"0x61fb9e7583dc92cc76eb52f93d88addaa0f02265",)"
                 R"("markPx":"80.498",)"
                 R"("method":"market")"
                 R"(},)"
                 R"("feeToken":"USDC",)"
                 R"("twapId":null)"
                 R"(},{)"
                 R"("coin":"xyz:SILVER",)"
                 R"("px":"82.739",)"
                 R"("sz":"0.3",)"
                 R"("side":"B",)"
                 R"("time":1769980172571,)"
                 R"("startPosition":"0.3",)"
                 R"("dir":"Open Long",)"
                 R"("closedPnl":"0.0",)"
                 R"("hash":"0x2915c3d9f3bda6db2a8f04347e018b0202d400bf8eb0c5adccde6f2cb2b180c5",)"
                 R"("oid":307033890639,)"
                 R"("crossed":false,)"
                 R"("fee":"0.000714",)"
                 R"("tid":449934974045765,)"
                 R"("cloid":"0x508000013026000ff97db8b00002787b",)"
                 R"("feeToken":"USDC",)"
                 R"("twapId":null)"
                 R"(},{)"
                 R"("coin":"xyz:SILVER",)"
                 R"("px":"82.744",)"
                 R"("sz":"0.3",)"
                 R"("side":"B",)"
                 R"("time":1769980131295,)"
                 R"("startPosition":"0.0",)"
                 R"("dir":"Open Long",)"
                 R"("closedPnl":"0.0",)"
                 R"("hash":"0xd088bfdef5bbec60d20204347dff8f020ae000c490bf0b3274516b31b4bfc64b",)"
                 R"("oid":307033477544,)"
                 R"("crossed":false,)"
                 R"("fee":"0.000714",)"
                 R"("tid":762172091049847,)"
                 R"("cloid":"0x508000013026000baaf2c90000019605",)"
                 R"("feeToken":"USDC",)"
                 R"("twapId":null)"
                 R"(},{)"
                 R"("coin":"BTC",)"
                 R"("px":"81623.0",)"
                 R"("sz":"0.00031",)"
                 R"("side":"B",)"
                 R"("time":1769869597418,)"
                 R"("startPosition":"0.0003",)"
                 R"("dir":"Open Long",)"
                 R"("closedPnl":"0.0",)"
                 R"("hash":"0xaa83fdbee40c4a98abfd043469198e0211d200a47f0f696a4e4ca911a3002483",)"
                 R"("oid":307033892448,)"
                 R"("crossed":false,)"
                 R"("fee":"0.003643",)"
                 R"("tid":736187409879903,)"
                 R"("cloid":"0x508000013026000ff97db8b00001787b",)"
                 R"("feeToken":"USDC",)"
                 R"("twapId":null)"
                 R"(},{)"
                 R"("coin":"BTC",)"
                 R"("px":"83959.0",)"
                 R"("sz":"0.0003",)"
                 R"("side":"B",)"
                 R"("time":1769814589170,)"
                 R"("startPosition":"0.0",)"
                 R"("dir":"Open Long",)"
                 R"("closedPnl":"0.0",)"
                 R"("hash":"0x7776e4d1b86ebb1a78f004345ebcd70208db00b75361d9ec1b3f902477629505",)"
                 R"("oid":307033479126,)"
                 R"("crossed":false,)"
                 R"("fee":"0.003627",)"
                 R"("tid":606867627195097,)"
                 R"("cloid":"0x508000013026000baaf2c90000029605",)"
                 R"("feeToken":"USDC",)"
                 R"("twapId":null)"
                 R"(},{)"
                 R"("coin":"@151",)"
                 R"("px":"3414.3",)"
                 R"("sz":"0.000026746",)"
                 R"("side":"A",)"
                 R"("time":1762992000007,)"
                 R"("startPosition":"0.000026746",)"
                 R"("dir":"Spot Dust Conversion",)"
                 R"("closedPnl":"0.0",)"
                 R"("hash":"0x0000000000000000000000000000000000000000000000000000000000000000",)"
                 R"("oid":232818693775,)"
                 R"("crossed":true,)"
                 R"("fee":"0.0",)"
                 R"("tid":0,)"
                 R"("feeToken":"USDC",)"
                 R"("twapId":null)"
                 R"(},{)"
                 R"("coin":"@151",)"
                 R"("px":"3483.6",)"
                 R"("sz":"0.0526",)"
                 R"("side":"A",)"
                 R"("time":1762961067165,)"
                 R"("startPosition":"0.052626746",)"
                 R"("dir":"Sell",)"
                 R"("closedPnl":"0.09994001",)"
                 R"("hash":"0x477e979f5eb2d51a48f8042f584dcf0208b00084f9b5f3eceb4742f21db6af04",)"
                 R"("oid":232375809344,)"
                 R"("crossed":true,)"
                 R"("fee":"0.1231355",)"
                 R"("tid":372973698966409,)"
                 R"("feeToken":"USDC",)"
                 R"("twapId":null)"
                 R"(})"
                 R"(])";
  auto helper = [&](value_type &obj) {
    REQUIRE(std::size(obj.data) == 7);
    /*
    auto &o0 = obj.data[0];
    CHECK(o0.coin == "ETH"sv);
    CHECK(o0.side == "B"sv);
    CHECK(o0.limit_px == 1800.0_a);
    CHECK(o0.sz == 1.0_a);
    CHECK(o0.oid == 311642020084);
    CHECK(o0.timestamp == 1770219647089ms);
    CHECK(o0.orig_sz == 1.0_a);
    CHECK(o0.cloid == "0x430882655000030000000001000000b9"sv);
    */
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
