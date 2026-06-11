/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::ActiveAssetCtx;

TEST_CASE("simple", "[json_active_asset_ctx]") {
  auto message = R"({)"
                 R"("channel":"activeAssetCtx",)"
                 R"("data":{)"
                 R"("coin":"SOL",)"
                 R"("ctx":{)"
                 R"("funding":"0.0000125",)"
                 R"("openInterest":"5259.34",)"
                 R"("prevDayPx":"132.32",)"
                 R"("dayNtlVlm":"965843.7286000003",)"
                 R"("premium":"0.0003761389",)"
                 R"("oraclePx":"128.41",)"
                 R"("markPx":"128.45",)"
                 R"("midPx":"128.48",)"
                 R"("impactPxs":["128.4583","128.5099"],)"
                 R"("dayBaseVlm":"7543.61")"
                 R"(})"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.channel == protocol::json::Channel::ACTIVE_ASSET_CTX);
    CHECK(obj.data.coin == "SOL"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
