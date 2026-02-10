/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#include "roq/utils/hash/keccak256.hpp"

#include "roq/hyperliquid/crypto/wallet.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct Crypto final {
  Crypto(std::string_view const &key, std::string_view const &secret);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  auto const &get_key() const { return key_; }

  std::string sign(std::string_view const &action, std::vector<uint8_t> const &packed, std::chrono::milliseconds now_utc);

 private:
  using Hash = utils::hash::Keccak256;
  using Digest = std::array<std::byte, Hash::DIGEST_LENGTH>;

  std::string const key_;
  crypto::Wallet wallet_;  // XXX FIXME TODO move to tools
  Digest digest_;
  Hash hash_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
