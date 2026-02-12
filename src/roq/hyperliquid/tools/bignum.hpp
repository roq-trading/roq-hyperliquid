/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <openssl/bn.h>

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

namespace roq {
namespace hyperliquid {
namespace tools {

struct BigNum final {
  using value_type = BIGNUM;

  BigNum();

  explicit BigNum(value_type *);
  explicit BigNum(value_type const *);

  explicit BigNum(BigNum const &);

  BigNum(BigNum &&) = default;

  void operator=(BigNum const &) = delete;

  operator value_type *() { return handle_.get(); }
  operator value_type const *() const { return handle_.get(); }

  bool empty() const;

  size_t size() const;

  int compare(BigNum const &) const;

  std::span<std::byte> to_binary(std::span<std::byte> const &) const;
  std::span<std::byte> to_binary_pad(std::span<std::byte> const &) const;

  static BigNum create_from_hex(std::string_view const &);
  static BigNum create_from_binary(std::span<std::byte const> const &);

 private:
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
