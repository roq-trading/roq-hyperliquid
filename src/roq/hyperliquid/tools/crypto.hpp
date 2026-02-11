/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <span>
#include <string>
#include <string_view>

#include "roq/utils/hash/keccak256.hpp"

#include "roq/hyperliquid/tools/wallet.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct Crypto final {
  Crypto(std::string_view const &key, std::string_view const &secret);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  auto const &get_key() const { return key_; }

  std::string sign_l1_action(
      std::string_view const &action, std::span<std::byte const> const &packed, std::chrono::milliseconds now_utc, std::chrono::milliseconds expires_after_utc);

 private:
  using Hash = utils::hash::Keccak256;
  using Digest = std::array<std::byte, Hash::DIGEST_LENGTH>;

  std::string const key_;
  bool const mainnet_;
  Wallet wallet_;
  Digest digest_;
  Hash hash_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
