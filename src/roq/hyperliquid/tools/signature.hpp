/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace roq {
namespace hyperliquid {
namespace tools {

struct Signature final {
  std::string r;  // hex string with "0x" prefix
  std::string s;  // hex string with "0x" prefix
  int v = {};     // recovery id (27 or 28)

  nlohmann::json toJson() const {
    return {
        {"r", r},
        {"s", s},
        {"v", v},
    };
  }
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
