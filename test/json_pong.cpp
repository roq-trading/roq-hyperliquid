/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::Pong;

TEST_CASE("simple", "[json_pong]") {
  auto message = R"({)"
                 R"("channel":"pong")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) { CHECK(obj.channel == protocol::json::Channel::PONG); };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
