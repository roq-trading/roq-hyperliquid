/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/bignum.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
using value_type = BigNum::value_type;

void deleter(value_type *ptr) {
  if (ptr != nullptr) {
    BN_free(ptr);
  }
}

template <typename R>
R create() {
  R result{BN_new(), deleter};
  if (!result) {
    throw RuntimeError{"BN_new"sv};
  }
  return result;
}

template <typename R>
R create(BIGNUM const *obj) {
  R result{BN_dup(obj), deleter};
  if (!result) {
    throw RuntimeError{"BN_dup"sv};
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

BigNum::BigNum() : handle_{create<decltype(handle_)>()} {
}

BigNum::BigNum(std::string_view const &private_key) : BigNum() {
  std::string key_hex{private_key};
  if (key_hex.substr(0, 2) == "0x") {
    key_hex = key_hex.substr(2);
  }
  auto ptr = handle_.get();
  if (BN_hex2bn(&ptr, key_hex.c_str()) == 0) {
    throw RuntimeError{"BN_hex2bn"sv};
  }
}

BigNum::BigNum(value_type const *obj) : handle_{create<decltype(handle_)>(obj)} {
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
