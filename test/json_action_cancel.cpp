/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::ActionCancel;

TEST_CASE("cancel_failure", "[json_action_cancel]") {
  auto message = R"({)"
                 R"("channel":"post",)"
                 R"("data":{)"
                 R"("id":3000003,)"
                 R"("response":{)"
                 R"("type":"action",)"
                 R"("payload":{)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"cancel",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("error":"Order was never placed, already canceled, or filled. asset=5")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})"
                 R"(})"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == protocol::json::Channel::POST);
    CHECK(obj.data.id == 3000003);
    CHECK(obj.data.response.type == protocol::json::ResponseType::ACTION);
    CHECK(obj.data.response.payload.status == protocol::json::Status::OK);
    CHECK(obj.data.response.payload.response.type == protocol::json::ResponseType::CANCEL);
    //
    CHECK(obj.data.response.payload.response.data.statuses == R"([{"error":"Order was never placed, already canceled, or filled. asset=5"}])"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}

TEST_CASE("cancel_success", "[json_action_cancel]") {
  auto message = R"({)"
                 R"("channel":"post",)"
                 R"("data":{)"
                 R"("id":3000002,)"
                 R"("response":{)"
                 R"("type":"action",)"
                 R"("payload":{)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"cancel",)"
                 R"("data":{)"
                 R"("statuses":[)"
                 R"("success")"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})"
                 R"(})"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == protocol::json::Channel::POST);
    CHECK(obj.data.id == 3000002);
    CHECK(obj.data.response.type == protocol::json::ResponseType::ACTION);
    CHECK(obj.data.response.payload.status == protocol::json::Status::OK);
    CHECK(obj.data.response.payload.response.type == protocol::json::ResponseType::CANCEL);
    //
    CHECK(obj.data.response.payload.response.data.statuses == R"(["success"])"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
