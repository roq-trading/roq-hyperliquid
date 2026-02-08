/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/create_order_ack.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::CreateOrderAck;

TEST_CASE("success_created", "[json_create_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"order",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("resting":{)"
                 R"("oid":311642020084,)"
                 R"("cloid":"0x430882655000030000000001000000b9")"
                 R"(})"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.resting.oid == 311642020084);
    CHECK(s0.resting.cloid == "0x430882655000030000000001000000b9"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("success_filled", "[json_create_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"order",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("filled":{"totalSz":"0.01","avgPx":"2080.2","oid":315445859529,"cloid":"0x000300511abd579f000001000000007b")"
                 R"(})"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.filled.total_sz == 0.01_a);
    CHECK(s0.filled.avg_px == 2080.2_a);
    CHECK(s0.filled.oid == 315445859529);
    CHECK(s0.filled.cloid == "0x000300511abd579f000001000000007b"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("failure_price", "[json_create_order_ack]") {
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
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.error == "Order price cannot be more than 95% away from the reference price"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("failure_quantity", "[json_create_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"order",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("error":"Order must have minimum value of $10. asset=1")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.error == "Order must have minimum value of $10. asset=1"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
