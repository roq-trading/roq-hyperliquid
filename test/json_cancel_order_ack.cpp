/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/cancel_order_ack.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using value_type = json::CancelOrderAck;

TEST_CASE("failure_by_oid", "[json_cancel_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"cancel",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("error":"Order was never placed, already canceled, or filled. asset=0")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.status == "ok"sv);
    CHECK(obj.response.type == "cancel"sv);
    // REQUIRE(std::size(obj.response.data.statuses) == 1);
    // CHECK(obj.response.data.statuses[0] == "success"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
TEST_CASE("success_by_oid", "[json_cancel_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"cancel",)"
                 R"("data":{)"
                 R"("statuses":[)"
                 R"("success")"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.status == "ok"sv);
    CHECK(obj.response.type == "cancel"sv);
    // REQUIRE(std::size(obj.response.data.statuses) == 1);
    // CHECK(obj.response.data.statuses[0] == "success"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
