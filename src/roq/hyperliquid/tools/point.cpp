/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/point.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
using value_type = Point::value_type;

void deleter(value_type *ptr) {
  if (ptr != nullptr) {
    EC_POINT_free(ptr);
  }
}

template <typename R>
R create(auto group) {
  R result{EC_POINT_new(group), deleter};
  if (!result) {
    throw RuntimeError{"EC_POINT_new"sv};
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Point::Point(EC_GROUP const *group) : handle_{create<decltype(handle_)>(group)} {
}

void Point::multiply(EC_GROUP const *group, BIGNUM const *n, EC_POINT const *q, BIGNUM const *m, BN_CTX *ctx) {
  if (EC_POINT_mul(group, handle_.get(), n, q, m, ctx) != 1) {
    throw RuntimeError{"EC_POINT_mul"sv};
  }
}

void Point::get_affine_coordinates(EC_GROUP const *group, BIGNUM *x, BIGNUM *y, BN_CTX *ctx) {
  if (EC_POINT_get_affine_coordinates(group, handle_.get(), x, y, ctx) != 1) {
    throw RuntimeError{"EC_POINT_get_affine_coordinates"sv};
  }
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
