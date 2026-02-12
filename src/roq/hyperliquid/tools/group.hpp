/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <openssl/ec.h>

#include <memory>

#include "roq/hyperliquid/tools/bignum.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct Group final {
  using value_type = EC_GROUP;

  explicit Group(value_type *);
  explicit Group(value_type const *);

  operator value_type *() { return handle_.get(); }
  operator value_type const *() const { return handle_.get(); }

  BigNum get_order() const;
  BigNum get_curve() const;

 private:
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
