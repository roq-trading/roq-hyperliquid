/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/bignum.hpp"

#include "roq/exceptions.hpp"

#include "roq/utils/safe_cast.hpp"

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

void noop_deleter(value_type *) {
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
R create(auto handle) {
  R result{handle, deleter};
  return result;
}

template <typename R>
R create_2(auto handle) {
  R result{handle, noop_deleter};
  return result;
}

template <typename R>
R duplicate(auto handle) {
  R result{handle, noop_deleter};
  return result;
}

}  // namespace

// === IMPLEMENTATION ===

BigNum::BigNum() : handle_{create<decltype(handle_)>()} {
}

BigNum::BigNum(value_type *handle) : handle_{create<decltype(handle_)>(const_cast<value_type *>(handle))} {
}

BigNum::BigNum(value_type const *handle) : handle_{create_2<decltype(handle_)>(const_cast<value_type *>(handle))} {
}

BigNum::BigNum(BigNum const &rhs) : BigNum{BN_dup(rhs.handle_.get())} {
}

bool BigNum::empty() const {
  return BN_is_zero(handle_.get());
}

size_t BigNum::size() const {
  auto result = BN_num_bytes(handle_.get());
  return utils::safe_cast(result);
}

int BigNum::compare(BigNum const &rhs) const {
  return BN_cmp(*this, rhs);
}

std::span<std::byte> BigNum::to_binary(std::span<std::byte> const &buffer) const {
  // XXX FIXME TODO std::size(*this) <= std::size(buffer)
  auto bytes = BN_bn2bin(handle_.get(), reinterpret_cast<unsigned char *>(std::data(buffer)));
  return {std::data(buffer), utils::safe_cast(bytes)};
}

std::span<std::byte> BigNum::to_binary_pad(std::span<std::byte> const &buffer) const {
  // XXX FIXME TODO std::size(*this) <= std::size(buffer)
  auto bytes = BN_bn2binpad(handle_.get(), reinterpret_cast<unsigned char *>(std::data(buffer)), utils::safe_cast(std::size(buffer)));
  if (bytes == -1) {
    throw RuntimeError{"BN_bn2binpad"sv};
  }
  return {std::data(buffer), utils::safe_cast(bytes)};
}

BigNum BigNum::create_from_hex(std::string_view const &value) {
  std::string key_hex{value};  // XXX FIXME TODO this is not great...
  if (key_hex.substr(0, 2) == "0x"sv) {
    key_hex = key_hex.substr(2);
  }
  BigNum result;
  auto handle = result.handle_.get();
  if (BN_hex2bn(&handle, key_hex.c_str()) == 0) {
    throw RuntimeError{"BN_hex2bn"sv};
  }
  return result;
}

BigNum BigNum::create_from_binary(std::span<std::byte const> const &value) {
  BigNum result;
  if (BN_bin2bn(reinterpret_cast<unsigned char const *>(std::data(value)), std::size(value), result) == nullptr) {
    throw RuntimeError{"BN_bin2bn"sv};
  }
  return result;
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
