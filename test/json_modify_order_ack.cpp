/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/modify_order_ack.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using value_type = json::ModifyOrderAck;

TEST_CASE("failure", "[json_modify_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"order",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("error":"Attempted to modify to invalid new order")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    // REQUIRE(std::size(obj.response.data.statuses) == 1);
    // CHECK(obj.response.data.statuses[0] == "success"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
