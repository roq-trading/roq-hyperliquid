/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <openssl/evp.h>

#include <memory>

#include "roq/hyperliquid/tools/bignum.hpp"
#include "roq/hyperliquid/tools/group.hpp"
#include "roq/hyperliquid/tools/point.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct PKey final {
  using value_type = EVP_PKEY;

  explicit PKey(value_type *);
  explicit PKey(value_type const *);

  operator value_type *() { return handle_.get(); }
  operator value_type const *() const { return handle_.get(); }

  Group get_group() const;
  BigNum get_private_key() const;
  Point get_public_key() const;

  static PKey create_from_private_key(std::string_view const &);

 private:
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
