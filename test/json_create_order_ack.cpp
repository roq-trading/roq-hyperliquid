/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/create_order_ack.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using value_type = json::CreateOrderAck;

TEST_CASE("failure", "[json_create_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"order",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("error":"Order price cannot be more than 95% away from the reference price")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.status == "ok"sv);
    CHECK(obj.response.type == "order"sv);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.error == "Order price cannot be more than 95% away from the reference price"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
