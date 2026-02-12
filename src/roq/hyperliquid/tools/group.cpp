/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/group.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
using value_type = Group::value_type;

void deleter(value_type *ptr) {
  if (ptr != nullptr) {
    EC_GROUP_free(ptr);
  }
}

void noop_deleter(value_type *) {
}

template <typename R>
R create(auto value) {
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

Group::Group(value_type *value) : handle_{create<decltype(handle_)>(value)} {
}

Group::Group(value_type const *value) : handle_{create_2<decltype(handle_)>(const_cast<value_type *>(value))} {
}

BigNum Group::get_order() const {
  auto handle = EC_GROUP_get0_order(handle_.get());
  BigNum result{handle};
  return result;
}

BigNum Group::get_curve() const {
  BigNum p;
  EC_GROUP_get_curve(handle_.get(), p, nullptr, nullptr, nullptr);
  return p;
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
