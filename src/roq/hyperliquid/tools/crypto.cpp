/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/crypto.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &key, std::string_view const &secret) : key_{key}, wallet_{secret} {
  log::warn("DEBUG wallet address={}"sv, wallet_.address());
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
