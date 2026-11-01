/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/protocol/json/get_order_status_ack.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::GetOrderStatusAck;

TEST_CASE("simple", "[json_get_order_status_ack]") {
  auto message = R"({)"
                 R"("status":"order",)"
                 R"("order":{)"
                 R"("order":{)"
                 R"("coin":"SOL",)"
                 R"("side":"B",)"
                 R"("limitPx":"50.0",)"
                 R"("sz":"1.0",)"
                 R"("oid":591955049954,)"
                 R"("timestamp":1793543563021,)"
                 R"("triggerCondition":"N/A",)"
                 R"("isTrigger":false,)"
                 R"("triggerPx":"0.0",)"
                 R"("children":[],)"
                 R"("isPositionTpsl":false,)"
                 R"("reduceOnly":false,)"
                 R"("orderType":"Limit",)"
                 R"("origSz":"1.0",)"
                 R"("tif":"Gtc",)"
                 R"("cloid":"0x00030086b3a54198000001000000000c")"
                 R"(},)"
                 R"("status":"open",)"
                 R"("statusTimestamp":1793543563021})"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.status == "order"sv);
    CHECK(obj.order.order.coin == "SOL"sv);
    CHECK(obj.order.status == protocol::json::OrderStatus::OPEN);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("canceled", "[json_get_order_status_ack]") {
  auto message = R"({)"
                 R"("status":"order",)"
                 R"("order":{)"
                 R"("order":{)"
                 R"("coin":"SOL",)"
                 R"("side":"B",)"
                 R"("limitPx":"50.0",)"
                 R"("sz":"1.0",)"
                 R"("oid":591953864517,)"
                 R"("timestamp":1793543350525,)"
                 R"("triggerCondition":"N/A",)"
                 R"("isTrigger":false,)"
                 R"("triggerPx":"0.0",)"
                 R"("children":[],)"
                 R"("isPositionTpsl":false,)"
                 R"("reduceOnly":false,)"
                 R"("orderType":"Limit",)"
                 R"("origSz":"1.0",)"
                 R"("tif":"Gtc",)"
                 R"("cloid":"0x00030086b384a51c000001000000004d")"
                 R"(},)"
                 R"("status":"canceled",)"
                 R"("statusTimestamp":1793545283764)"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.status == "order"sv);
    CHECK(obj.order.order.coin == "SOL"sv);
    CHECK(obj.order.status == protocol::json::OrderStatus::CANCELED);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
