/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::ActionError;

TEST_CASE("create_failure_20260823", "[json_action_error]") {
  auto message = R"({)"
                 R"("channel":"post",)"
                 R"("data":{)"
                 R"("id":131561156372993,)"
                 R"("response":{)"
                 R"("type":"action",)"
                 R"("payload":{)"
                 R"("status":"err",)"
                 R"("response":"User or API Wallet 0x1234 does not exist.")"
                 R"(})"
                 R"(})"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == protocol::json::Channel::POST);
    CHECK(obj.data.id == 131561156372993);
    CHECK(obj.data.response.type == protocol::json::ResponseType::ACTION);
    CHECK(obj.data.response.payload.status == protocol::json::Status::ERR);
    CHECK(obj.data.response.payload.response == "User or API Wallet 0x1234 does not exist."sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
