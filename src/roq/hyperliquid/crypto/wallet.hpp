/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "roq/hyperliquid/crypto/signing.hpp"

namespace roq {
namespace hyperliquid {
namespace crypto {

class Wallet {
 public:
  // Create wallet from hex private key (with or without "0x" prefix)
  // static std::shared_ptr<Wallet> fromPrivateKey(std::string const &private_key_hex);
  explicit Wallet(std::string_view const &private_key_hex);

  // Get the Ethereum address derived from this wallet's public key
  std::string address() const;

  // Sign a message hash with ECDSA
  Signature signMessage(std::vector<uint8_t> const &message_hash) const;

  ~Wallet();

 private:
  explicit Wallet(void *ec_key);  // EC_KEY* hidden from header

  void *ec_key_;  // OpenSSL EC_KEY*
  std::string address_;
};

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
