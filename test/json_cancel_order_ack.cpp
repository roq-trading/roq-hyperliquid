/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/protocol/json/cancel_order_ack.hpp"
#include "roq/hyperliquid/protocol/json/cancel_order_ack_parser.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using value_type = protocol::json::CancelOrderAck;

/*
status
error
statuses[]:
  error->string
  success->{}
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
  auto result = protocol::json::CancelOrderAckParser::dispatch(message, buffers, trace_info, false, error_handler_2, success_handler_2);
  CHECK(result == true);
  return found;
};
}  // namespace

// === IMPLEMENTATION ===

TEST_CASE("already_canceled_or_filled", "[json_cancel_order_ack]") {
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
  auto error_handler = []([[maybe_unused]] auto const &text) { FAIL(); };
  auto success_handler = [](value_type const &obj) {
    CHECK(obj.status == protocol::json::Status::OK);
    CHECK(obj.response.type == protocol::json::ResponseType::CANCEL);
    REQUIRE(std::size(obj.response.data.statuses) == 1);
    CHECK(obj.response.data.statuses[0].error == "Order was never placed, already canceled, or filled. asset=0"sv);
  };
  auto found = helper(message, error_handler, success_handler);
  CHECK(found == true);
}

TEST_CASE("success", "[json_cancel_order_ack]") {
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
  auto error_handler = []([[maybe_unused]] auto const &text) { FAIL(); };
  auto success_handler = [](value_type const &) { FAIL(); };
  auto found = helper(message, error_handler, success_handler);
  CHECK(found == false);  // note! we can normally get away with not ack'ing success
}
