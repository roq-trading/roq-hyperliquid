/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <memory>
#include <string_view>

#include <openssl/bn.h>

namespace roq {
namespace hyperliquid {
namespace tools {

struct BigNum final {
  using value_type = BIGNUM;

  BigNum();

  explicit BigNum(std::string_view const &private_key);

  explicit BigNum(value_type const *);

  operator value_type *() { return handle_.get(); }
  operator value_type const *() const { return handle_.get(); }

 private:
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
