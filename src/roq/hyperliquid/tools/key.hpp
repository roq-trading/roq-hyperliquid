/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <memory>
#include <string_view>

#include <openssl/ec.h>

namespace roq {
namespace hyperliquid {
namespace tools {

struct Key final {
  using value_type = EC_KEY;

  explicit Key(std::string_view const &private_key);

  operator EC_GROUP const *() const;

  BIGNUM const *get_private_key() const;
  EC_POINT const *get_public_key() const;

  std::string derive_address() const;

 private:
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
