/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/key.hpp"

#include <vector>

#include <openssl/evp.h>
#include <openssl/obj_mac.h>

#include "roq/exceptions.hpp"

#include "roq/hyperliquid/tools/bignum.hpp"
#include "roq/hyperliquid/tools/keccak256.hpp"
#include "roq/hyperliquid/tools/point.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
using value_type = Key::value_type;

void deleter(value_type *ptr) {
  if (ptr != nullptr) {
    EC_KEY_free(ptr);
  }
}

void set_private_key(auto &handle, auto &bignum) {
  if (EC_KEY_set_private_key(handle.get(), bignum) != 1) {
    throw RuntimeError{"EC_KEY_set_private_key"sv};
  }
}

auto get_group(auto &handle) {
  return EC_KEY_get0_group(handle.get());
}

auto set_public_key(auto &handle, auto &point) {
  if (EC_KEY_set_public_key(handle.get(), point) != 1) {
    throw RuntimeError{"EC_KEY_set_public_key"sv};
  }
}

void validate(auto &handle) {
  if (EC_KEY_check_key(handle.get()) != 1) {
    throw RuntimeError{"EC_KEY_check_key"sv};
  }
}

template <typename R>
R create(auto &private_key) {
  R result{EC_KEY_new_by_curve_name(NID_secp256k1), deleter};
  if (!result) {
    throw RuntimeError{"EC_KEY_new_by_curve_name"sv};
  }
  BigNum bignum{private_key};
  set_private_key(result, bignum);
  auto group = get_group(result);
  Point public_key{group};
  public_key.multiply(group, bignum, nullptr, nullptr);
  set_public_key(result, public_key);
  validate(result);
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Key::Key(std::string_view const &private_key) : handle_{create<decltype(handle_)>(private_key)} {
}

Key::operator EC_GROUP const *() const {
  auto result = EC_KEY_get0_group(handle_.get());
  if (!result) {
    throw RuntimeError{"EC_KEY_get0_group"sv};
  }
  return result;
}

BIGNUM const *Key::get_private_key() const {
  auto result = EC_KEY_get0_private_key(handle_.get());
  if (!result) {
    throw RuntimeError{"EC_KEY_get0_private_key"sv};
  }
  return result;
}

EC_POINT const *Key::get_public_key() const {
  auto result = EC_KEY_get0_public_key(handle_.get());
  if (!result) {
    throw RuntimeError{"EC_KEY_get0_public_key"sv};
  }
  return result;
}

std::string Key::derive_address() const {
  auto group = EC_KEY_get0_group(handle_.get());
  auto pub_key = EC_KEY_get0_public_key(handle_.get());

  std::vector<uint8_t> pub_key_bytes(65);
  size_t len = EC_POINT_point2oct(group, pub_key, POINT_CONVERSION_UNCOMPRESSED, pub_key_bytes.data(), 65, nullptr);

  if (len != 65) {
    throw std::runtime_error("Failed to convert public key");
  }

  auto hash = Keccak256::keccak256({pub_key_bytes.data() + 1, 64});

  std::string address = "0x";
  for (size_t i = 12; i < 32; ++i) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%02x", hash[i]);
    address += buf;
  }

  return address;
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
