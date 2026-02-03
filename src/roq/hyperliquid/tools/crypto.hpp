/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/hyperliquid/crypto/wallet.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct Crypto final {
  Crypto(std::string_view const &key, std::string_view const &secret);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  operator crypto::Wallet &() { return wallet_; }

  auto const &get_key() const { return key_; }

 private:
  std::string const key_;
  crypto::Wallet wallet_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
