/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <memory>

#include <openssl/bn.h>
#include <openssl/ec.h>

namespace roq {
namespace hyperliquid {
namespace tools {

struct Point final {
  using value_type = EC_POINT;

  explicit Point(EC_GROUP const *);

  operator value_type *() { return handle_.get(); }
  operator value_type const *() const { return handle_.get(); }

  void multiply(EC_GROUP const *, BIGNUM const *n, EC_POINT const *q, BIGNUM const *m, BN_CTX * = nullptr);

  void get_affine_coordinates(EC_GROUP const *, BIGNUM *x, BIGNUM *y, BN_CTX * = nullptr);

 private:
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
