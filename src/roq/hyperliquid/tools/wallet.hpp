/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <span>
#include <string>
#include <string_view>

#include "roq/hyperliquid/tools/key.hpp"
#include "roq/hyperliquid/tools/signature.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct Wallet final {
  Wallet(std::string_view const &private_key);

  Wallet(Wallet &&) = delete;
  Wallet(Wallet const &) = delete;

  std::string_view get_address() const { return address_; }

  Signature sign_message(std::span<uint8_t const> const &hash) const;

 private:
  Key const key_;
  std::string const address_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
