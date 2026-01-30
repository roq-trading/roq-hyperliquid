/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace roq {
namespace hyperliquid {
namespace tools {

struct Keccak256 final {
  static std::vector<uint8_t> keccak256(std::span<uint8_t const> const &);
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
