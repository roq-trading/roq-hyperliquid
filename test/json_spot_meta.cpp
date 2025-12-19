/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::hyperliquid;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::SpotMeta;

TEST_CASE("simple", "[json_spot_meta]") {
  auto message = R"({)"
                 R"("channel":"post",)"
                 R"("data":{)"
                 R"("id":2000001,)"
                 R"("response":{)"
                 R"("type":"info",)"
                 R"("payload":{)"
                 R"("type":"spotMeta",)"
                 R"("data":{)"
                 R"("universe":[{)"
                 R"("tokens":[1,0],)"
                 R"("name":"PURR/USDC",)"
                 R"("index":0,)"
                 R"("isCanonical":true)"
                 R"(},{)"
                 R"("tokens":[2,0],)"
                 R"("name":"@1",)"
                 R"("index":1,)"
                 R"("isCanonical":false)"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})"
                 R"(})"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) { CHECK(obj.channel == json::Channel::POST); };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
