/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "roq/hyperliquid/tools/key.hpp"
#include "roq/hyperliquid/tools/signature.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct Wallet final {
  Wallet(std::string_view const &private_key);

  Wallet(Wallet &&) = delete;
  Wallet(Wallet const &) = delete;

  operator Key const &() const { return key_; }

  std::string_view get_address() const { return address_; }

  //

  std::vector<uint8_t> action_hash(
      int action, std::string_view const &vault_address, std::chrono::milliseconds nonce, std::chrono::milliseconds expires_after) const;

  int construct_phantom_agent(std::vector<uint8_t> const &hash, bool is_mainnet) const;

  int l1_payload(int phantom_agent) const;

  std::vector<uint8_t> encode_typed_data(int typed_data) const;

  Signature sign_message(std::span<uint8_t const> const &hash) const;

  Signature sign_l1_action(
      int action, std::string_view const &vault_address, std::chrono::milliseconds nonce, std::chrono::milliseconds expires_after, bool is_mainnet) const;

 private:
  Key const key_;
  std::string const address_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
