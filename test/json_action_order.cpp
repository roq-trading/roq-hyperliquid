/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::ActionOrder;

TEST_CASE("create_failure", "[json_action_order]") {
  auto message = R"({)"
                 R"("channel":"post",)"
                 R"("data":{)"
                 R"("id":3000001,)"
                 R"("response":{)"
                 R"("type":"action",)"
                 R"("payload":{)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"order",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("error":"Order must have minimum value of $10. asset=5")"
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
    CHECK(obj.data.id == 3000001);
    CHECK(obj.data.response.type == protocol::json::ResponseType::ACTION);
    CHECK(obj.data.response.payload.status == protocol::json::Status::OK);
    CHECK(obj.data.response.payload.response.type == protocol::json::ResponseType::ORDER);
    //
    REQUIRE(std::size(obj.data.response.payload.response.data.statuses) == 1);
    auto &s0 = obj.data.response.payload.response.data.statuses[0];
    CHECK(s0.error == R"(Order must have minimum value of $10. asset=5)"sv);
    CHECK(s0.resting.oid == 0);
    CHECK(std::empty(s0.resting.cloid));
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}

TEST_CASE("create_success", "[json_action_order]") {
  auto message = R"({)"
                 R"("channel":"post",)"
                 R"("data":{)"
                 R"("id":3000001,)"
                 R"("response":{)"
                 R"("type":"action",)"
                 R"("payload":{)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"order",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("resting":{)"
                 R"("oid":496160097503,)"
                 R"("cloid":"0x00030070b4ed5113000001000000002e")"
                 R"(})"
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
    CHECK(obj.data.id == 3000001);
    CHECK(obj.data.response.type == protocol::json::ResponseType::ACTION);
    CHECK(obj.data.response.payload.status == protocol::json::Status::OK);
    CHECK(obj.data.response.payload.response.type == protocol::json::ResponseType::ORDER);
    //
    REQUIRE(std::size(obj.data.response.payload.response.data.statuses) == 1);
    auto &s0 = obj.data.response.payload.response.data.statuses[0];
    CHECK(std::empty(s0.error));
    CHECK(s0.resting.oid == 496160097503);
    CHECK(s0.resting.cloid == "0x00030070b4ed5113000001000000002e"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
