/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/key.hpp"

#include <openssl/evp.h>

#include <vector>

#include "roq/exceptions.hpp"

#include "roq/utils/hash/keccak256.hpp"

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

Key::Key(value_type *value) : handle_{create_1<decltype(handle_)>(value)} {
}

Key::Key(value_type const *value) : handle_{create_2<decltype(handle_)>(const_cast<value_type *>(value))} {
}

Group Key::get_group() const {
  auto handle = EC_KEY_get0_group(handle_.get());
  if (!handle) {
    throw RuntimeError{"EC_KEY_get0_group"sv};
  }
  Group result{handle};
  return result;
}

BigNum Key::get_private_key() const {
  auto handle = EC_KEY_get0_private_key(handle_.get());
  if (!handle) {
    throw RuntimeError{"EC_KEY_get0_private_key"sv};
  }
  BigNum result{handle};
  return result;
}

Point Key::get_public_key() const {
  auto handle = EC_KEY_get0_public_key(handle_.get());
  if (!handle) {
    throw RuntimeError{"EC_KEY_get0_public_key"sv};
  }
  Point result{handle};
  return result;
}

void Key::set_private_key(BigNum const &private_key) {
  if (EC_KEY_set_private_key(handle_.get(), private_key) != 1) {
    throw RuntimeError{"EC_KEY_set_private_key"sv};
  }
}

void Key::set_public_key(Point const &public_key) {
  if (EC_KEY_set_public_key(handle_.get(), public_key) != 1) {
    throw RuntimeError{"EC_KEY_set_public_key"sv};
  }
}

void Key::validate() const {
  if (EC_KEY_check_key(handle_.get()) != 1) {
    throw RuntimeError{"EC_KEY_check_key"sv};
  }
}

std::string Key::derive_address() const {
  auto group = get_group();
  auto pub_key = get_public_key();

  std::vector<uint8_t> pub_key_bytes(65);
  size_t len = EC_POINT_point2oct(group, pub_key, POINT_CONVERSION_UNCOMPRESSED, std::data(pub_key_bytes), 65, nullptr);

  if (len != 65) {
    throw RuntimeError{"Failed to convert public key"};
  }

  utils::hash::Keccak256 hash;  // XXX FIXME TODO re-use
  std::span payload{std::data(pub_key_bytes) + 1, 64};
  std::vector<std::byte> digest(32);
  hash.clear();
  hash.update(payload);
  hash.final(digest);

  std::string address = "0x";
  for (size_t i = 12; i < 32; ++i) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%02x", static_cast<int>(digest[i]));
    address += buf;
  }

  return address;
}

Key Key::create_from_private_key(std::string_view const &private_key) {
  auto handle = EC_KEY_new_by_curve_name(NID_secp256k1);
  if (!handle) {
    throw RuntimeError{"EC_KEY_new_by_curve_name"sv};
  }
  Key result{handle};
  auto bignum = BigNum::create_from_hex(private_key);
  result.set_private_key(bignum);
  auto group = result.get_group();
  auto public_key = Point::create_from_group(group);
  public_key.multiply(group, bignum, nullptr, nullptr);
  result.set_public_key(public_key);
  result.validate();
  return result;
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
