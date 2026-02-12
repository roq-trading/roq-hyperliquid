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

void noop_deleter(value_type *) {
}

template <typename R>
R create_1(auto value) {
  R result{value, deleter};
  return result;
}

template <typename R>
R create_2(auto value) {
  R result{value, noop_deleter};
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Point::Point(value_type *value) : handle_{create_1<decltype(handle_)>(value)} {
}

Point::Point(value_type const *value) : handle_{create_2<decltype(handle_)>(const_cast<value_type *>(value))} {
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

Point Point::create_from_group(EC_GROUP const *group) {
  Point result{EC_POINT_new(group)};
  if (!result.handle_) {
    throw RuntimeError{"EC_POINT_new"sv};
  }
  return result;
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
