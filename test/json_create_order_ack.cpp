/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/hyperliquid/json/create_order_ack.hpp"
#include "roq/hyperliquid/json/create_order_ack_parser.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::CreateOrderAck;

/*
status
error
statuses[]:
  error->string
  resting->{oid,cloid}
  filled->{total/avg/oid,cloid}
*/

// === HELPERS ===

namespace {
auto helper = [](auto &message, auto error_handler, auto success_handler) {
  auto found = false;
  auto error_handler_2 = [&](auto const &text) {
    found = true;
    error_handler(text);
  };
  auto success_handler_2 = [&](auto &event) {
    found = true;
    success_handler(event);
  };
  core::json::BufferStack buffers{8192, 1};
  TraceInfo trace_info;
  auto result = json::CreateOrderAckParser::dispatch(message, buffers, trace_info, false, error_handler_2, success_handler_2);
  CHECK(result == true);
  CHECK(found == true);
};
}  // namespace

// === IMPLEMENTATION ===

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
  auto error_handler = []([[maybe_unused]] auto const &text) { FAIL(); };
  auto success_handler = [](value_type const &obj) {
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.resting.oid == 311642020084);
    CHECK(s0.resting.cloid == "0x430882655000030000000001000000b9"sv);
  };
  helper(message, error_handler, success_handler);
}

TEST_CASE("success_filled", "[json_create_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"order",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("filled":{)"
                 R"("totalSz":"0.01",)"
                 R"("avgPx":"2080.2",)"
                 R"("oid":315445859529,)"
                 R"("cloid":"0x000300511abd579f000001000000007b")"
                 R"(})"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})";
  auto error_handler = []([[maybe_unused]] auto const &text) { FAIL(); };
  auto success_handler = [](value_type const &obj) {
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.filled.total_sz == 0.01_a);
    CHECK(s0.filled.avg_px == 2080.2_a);
    CHECK(s0.filled.oid == 315445859529);
    CHECK(s0.filled.cloid == "0x000300511abd579f000001000000007b"sv);
  };
  helper(message, error_handler, success_handler);
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
  auto error_handler = []([[maybe_unused]] auto const &text) { FAIL(); };
  auto success_handler = [](value_type const &obj) {
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.error == "Order price cannot be more than 95% away from the reference price"sv);
  };
  helper(message, error_handler, success_handler);
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
  auto error_handler = []([[maybe_unused]] auto const &text) { FAIL(); };
  auto success_handler = [](value_type const &obj) {
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.error == "Order must have minimum value of $10. asset=1"sv);
  };
  helper(message, error_handler, success_handler);
}

TEST_CASE("ioc_failed", "[json_create_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"order",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("error":"Order could not immediately match against any resting orders. asset=5")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})";
  auto error_handler = []([[maybe_unused]] auto const &text) { FAIL(); };
  auto success_handler = [](value_type const &obj) {
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.error == "Order could not immediately match against any resting orders. asset=5"sv);
  };
  helper(message, error_handler, success_handler);
}

TEST_CASE("insufficient_margin", "[json_create_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("response":{)"
                 R"("type":"order",)"
                 R"("data":{)"
                 R"("statuses":[{)"
                 R"("error":"Insufficient margin to place order. asset=5")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})";
  auto error_handler = []([[maybe_unused]] auto const &text) { FAIL(); };
  auto success_handler = [](value_type const &obj) {
    CHECK(obj.status == json::Status::OK);
    CHECK(obj.response.type == json::ResponseType::ORDER);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    auto &s0 = obj.response.data.statuses[0];
    CHECK(s0.error == "Insufficient margin to place order. asset=5"sv);
  };
  helper(message, error_handler, success_handler);
}

TEST_CASE("general_failure", "[json_create_order_ack]") {
  auto message = R"({)"
                 R"("status":"err",)"
                 R"("response":"User or API Wallet 0x4b6c5b1287e38b78e61d5dbca726d5e267eda747 does not exist.")"
                 R"(})";
  auto error_handler = []([[maybe_unused]] auto const &text) {
    CHECK(text == "User or API Wallet 0x4b6c5b1287e38b78e61d5dbca726d5e267eda747 does not exist."sv);
  };
  auto success_handler = [](value_type const &) { FAIL(); };
  helper(message, error_handler, success_handler);
}
