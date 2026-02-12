/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/pkey.hpp"

#include <openssl/core_names.h>

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
using value_type = PKey::value_type;

void deleter(value_type *ptr) {
  if (ptr != nullptr) {
    EVP_PKEY_free(ptr);
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

PKey::PKey(value_type *value) : handle_{create_1<decltype(handle_)>(value)} {
}

PKey::PKey(value_type const *value) : handle_{create_2<decltype(handle_)>(const_cast<value_type *>(value))} {
}

Group PKey::get_group() const {
  throw RuntimeError{"NOT IMPLEMENTED"sv};
}

BigNum PKey::get_private_key() const {
  BigNum result;
  auto tmp = static_cast<BigNum::value_type *>(result);
  if (EVP_PKEY_get_bn_param(handle_.get(), OSSL_PKEY_PARAM_PRIV_KEY, &tmp) == 0) {
    throw RuntimeError{"EVP_PKEY_get_bn_param"sv};
  }
  return result;
}

Point PKey::get_public_key() const {
  throw RuntimeError{"NOT IMPLEMENTED"sv};
  // EVP_PKEY_get_raw_public_key ???
}

PKey PKey::create_from_private_key(std::string_view const &private_key) {
  throw RuntimeError{"NOT IMPLEMENTED"sv};
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
