/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/wallet.hpp"

#include "roq/hyperliquid/tools/ecdsa.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
template <typename R>
auto create_key(auto &private_key) {
  using result_type = std::remove_cvref_t<R>;
  auto result = result_type::create_from_private_key(private_key);
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Wallet::Wallet(std::string_view const &private_key) : key_{create_key<decltype(key_)>(private_key)}, address_{key_.derive_address()} {
}

std::string Wallet::sign_ecdsa(std::span<std::byte const> const &hash) const {
  return ECDSA::signHash(key_, hash);
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
