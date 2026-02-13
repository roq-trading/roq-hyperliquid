/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Notification;

TEST_CASE("resting_filled", "[json_notification]") {
  auto message = R"({)"
                 R"("channel":"notification",)"
                 R"("data":{)"
                 R"("notification":"Resting order filled: Bought 0.0063 ETH at $1907.6")"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == json::Channel::NOTIFICATION);
    CHECK(obj.data.notification == "Resting order filled: Bought 0.0063 ETH at $1907.6"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
