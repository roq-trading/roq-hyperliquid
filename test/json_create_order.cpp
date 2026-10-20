/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/hyperliquid/tools/crypto.hpp"
#include "roq/hyperliquid/tools/encoder.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

// === CONSTANTS ===

namespace {
auto const EXAMPLE_WALLET_ADDRESS = "0x742d35Cc6634C0532925a3b844Bc454e4438f44e"sv;
auto const EXAMPLE_SECRET_KEY = "0xafd74db37ed043344744715d5e5331d255139031efb3c5a6d36e2f1e403d1544"sv;
}  // namespace

// === IMPLEMENTATION ===

TEST_CASE("simple", "[json_create_order]") {
  auto create_order = CreateOrder{
      .account = {},
      .order_id = {},
      .exchange = {},
      .symbol = "SOL"sv,
      .side = Side::BUY,
      .position_effect = {},
      .margin_mode = {},
      .quantity_type = {},
      .max_show_quantity = NaN,
      .order_type = OrderType::LIMIT,
      .time_in_force = TimeInForce::GTC,
      .execution_instructions = {},
      .execution_destination = {},
      .request_template = {},
      .quantity = 1.23,
      .price = 56.789,
      .stop_price = NaN,
      .leverage = NaN,
      .routing_id = {},
      .strategy_id = {},
      .release_time_utc = {},
  };
  server::oms::Order order;
  server::oms::RefData ref_data;
  ref_data.external_security_id = 198;
  ref_data.quantity = {
      .increment = 0.01,
      .precision = Precision::_2,
  };
  ref_data.price = {
      .increment = 0.001,
      .precision = Precision::_3,
  };
  auto request_id = "123"sv;  // note! hex value (without the "0x" prefix)
  auto now_utc = 123ms;
  auto expires_after_utc = 234ms;
  tools::Crypto crypto{EXAMPLE_WALLET_ADDRESS, EXAMPLE_SECRET_KEY};
  auto [action, packed] = tools::Encoder::create_order(create_order, order, ref_data, request_id, now_utc, expires_after_utc);
  auto request = crypto.sign_l1_action(action, packed, now_utc, expires_after_utc);
  CHECK(
      request == R"({)"
                 R"("action":{)"
                 R"("type":"order",)"
                 R"("orders":[{)"
                 R"("a":198,)"
                 R"("b":true,)"
                 R"("p":"56.789",)"
                 R"("s":"1.23",)"
                 R"("r":false,)"
                 R"("t":{)"
                 R"("limit":{)"
                 R"("tif":"Gtc")"
                 R"(})"
                 R"(},)"
                 R"("c":"0x00000000000000000000000000000123")"
                 R"(})"
                 R"(],)"
                 R"("grouping":"na")"
                 R"(},)"
                 R"("nonce":123,)"
                 R"("signature":{)"
                 R"("v":28,)"
                 R"("r":"0xb7181dd1b09de894dfe23f4ab736252f8941c6c292dd670342598cd310b93535",)"
                 R"("s":"0x61e3f2c4a54afdaf77983b957b618c88e3a6b8ba78e08ea31d2db0d67038ef84")"
                 R"(},)"
                 R"("expiresAfter":234)"
                 R"(})"sv);
}
