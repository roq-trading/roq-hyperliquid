/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/wallet.hpp"

#include "roq/hyperliquid/tools/ecdsa.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {}  // namespace

// === IMPLEMENTATION ===

Wallet::Wallet(std::string_view const &private_key) : key_{private_key}, address_{key_.derive_address()} {
}

std::string Wallet::sign_ecdsa(std::span<std::byte const> const &hash) const {
  /*
  std::vector<uint8_t> tmp;
  for (auto b : hash) {
    tmp.emplace_back(static_cast<uint8_t>(b));
  }
  */
  auto key = reinterpret_cast<void const *>(static_cast<Key::value_type const *>(key_));
  return ECDSA::signHash(key, hash);
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
