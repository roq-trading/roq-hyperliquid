/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Trades;

TEST_CASE("simple", "[json_trades]") {
  auto message = R"({)"
                 R"("channel":"trades",)"
                 R"("data":[{)"
                 R"("coin":"SOL",)"
                 R"("side":"A",)"
                 R"("px":"128.5",)"
                 R"("sz":"0.42",)"
                 R"("time":1765887848009,)"
                 R"("hash":"0xab7ac612399342b1acf4041c880dcd011800ddf7d49661834f437164f8971c9c",)"
                 R"("tid":867891549538215,)"
                 R"("users":[)"
                 R"("0x768484f7e2ebb675c57838366c02ae99ba2a9b08",)"
                 R"("0xbb05602d74778f4a42dcee87d14051572bb9ec78")"
                 R"(])"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == json::Channel::TRADES);
    auto &data = obj.data;
    REQUIRE(std::size(data) == 1);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
