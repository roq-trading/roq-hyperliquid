/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::L2Book;

TEST_CASE("simple", "[json_l2_book]") {
  // truncated
  auto message = R"({)"
                 R"("channel":"l2Book",)"
                 R"("data":{)"
                 R"("coin":"BTC",)"
                 R"("time":1757130041083,)"
                 R"("levels":[[)"
                 R"({"px":"111065.0","sz":"6.98048","n":16},)"
                 R"({"px":"111064.0","sz":"0.54013","n":1},)"
                 R"({"px":"111063.0","sz":"5.49225","n":3},)"
                 R"({"px":"111062.0","sz":"0.03608","n":1},)"
                 R"({"px":"111060.0","sz":"0.00014","n":1},)"
                 R"({"px":"111059.0","sz":"3.17157","n":3},)"
                 R"({"px":"111058.0","sz":"0.00912","n":2},)"
                 R"({"px":"111057.0","sz":"0.06802","n":1},)"
                 R"({"px":"111056.0","sz":"1.56011","n":3},)"
                 R"({"px":"111055.0","sz":"1.48292","n":2},)"
                 R"({"px":"111054.0","sz":"0.95491","n":4},)"
                 R"({"px":"111053.0","sz":"0.58506","n":3},)"
                 R"({"px":"111052.0","sz":"5.16081","n":9},)"
                 R"({"px":"111051.0","sz":"0.77156","n":3},)"
                 R"({"px":"111050.0","sz":"1.89361","n":4},)"
                 R"({"px":"111049.0","sz":"0.82478","n":3},)"
                 R"({"px":"111048.0","sz":"8.01785","n":5},)"
                 R"({"px":"111047.0","sz":"2.03725","n":7},)"
                 R"({"px":"111046.0","sz":"2.09393","n":5},)"
                 R"({"px":"111045.0","sz":"0.97954","n":2})"
                 R"(],[)"
                 R"({"px":"111066.0","sz":"4.48632","n":21},)"
                 R"({"px":"111067.0","sz":"1.49503","n":8},)"
                 R"({"px":"111068.0","sz":"1.08038","n":3},)"
                 R"({"px":"111069.0","sz":"0.28616","n":4},)"
                 R"({"px":"111070.0","sz":"0.25372","n":4},)"
                 R"({"px":"111071.0","sz":"0.88206","n":5},)"
                 R"({"px":"111072.0","sz":"0.90608","n":4},)"
                 R"({"px":"111073.0","sz":"1.34418","n":2},)"
                 R"({"px":"111074.0","sz":"0.55144","n":3},)"
                 R"({"px":"111075.0","sz":"1.65341","n":5},)"
                 R"({"px":"111076.0","sz":"0.54019","n":2},)"
                 R"({"px":"111077.0","sz":"0.1151","n":2},)"
                 R"({"px":"111078.0","sz":"5.22221","n":6},)"
                 R"({"px":"111079.0","sz":"0.9401","n":4},)"
                 R"({"px":"111080.0","sz":"8.46586","n":8},)"
                 R"({"px":"111081.0","sz":"1.97488","n":6},)"
                 R"({"px":"111082.0","sz":"1.26367","n":5},)"
                 R"({"px":"111083.0","sz":"0.36031","n":4},)"
                 R"({"px":"111084.0","sz":"1.01393","n":4},)"
                 R"({"px":"111085.0","sz":"5.18286","n":4})"
                 R"(])"
                 R"(])"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == protocol::json::Channel::L2_BOOK);
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
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
