/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::SubscriptionResponse;

TEST_CASE("simple", "[json_subscription_response]") {
  auto message = R"({)"
                 R"("channel":"subscriptionResponse",)"
                 R"("data":{)"
                 R"("method":"subscribe",)"
                 R"("subscription":{)"
                 R"("type":"bbo",)"
                 R"("coin":"SOL")"
                 R"(})"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == json::Channel::SUBSCRIPTION_RESPONSE);
    CHECK(obj.data.method == "subscribe"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
