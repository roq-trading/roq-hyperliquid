/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::BBO;

TEST_CASE("simple", "[json_bbo]") {
  auto message = R"({)"
                 R"("channel":"bbo",)"
                 R"("data":{)"
                 R"("coin":"SOL",)"
                 R"("time":1765887848009,)"
                 R"("bbo":[{)"
                 R"("px":"128.5",)"
                 R"("sz":"36.93",)"
                 R"("n":2)"
                 R"(},{)"
                 R"("px":"128.54",)"
                 R"("sz":"18.7",)"
                 R"("n":2)"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == json::Channel::BBO);
    auto &bbo = obj.data.bbo;
    REQUIRE(std::size(bbo) == 2);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
