/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::Error;

TEST_CASE("simple", "[json_error]") {
  auto message = R"({)"
                 R"("channel":"error",)"
                 R"("data":"Error parsing JSON into valid websocket request: {\"method\":\"subscribe\",\"xsubscription\":{\"type\":\"bbo\",\"coin\":\"SOL\"}}")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == protocol::json::Channel::ERROR);
    CHECK(obj.data == R"(Error parsing JSON into valid websocket request: {\"method\":\"subscribe\",\"xsubscription\":{\"type\":\"bbo\",\"coin\":\"SOL\"}})"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
