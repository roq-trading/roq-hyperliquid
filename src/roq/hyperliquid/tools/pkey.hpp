/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <openssl/evp.h>

#include <memory>

#include "roq/utils/etherum/bignum.hpp"
#include "roq/utils/etherum/group.hpp"
#include "roq/utils/etherum/point.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct PKey final {
  using value_type = EVP_PKEY;

  explicit PKey(value_type *);
  explicit PKey(value_type const *);

  operator value_type *() { return handle_.get(); }
  operator value_type const *() const { return handle_.get(); }

  utils::etherum::Group get_group() const;
  utils::etherum::BigNum get_private_key() const;
  utils::etherum::Point get_public_key() const;

  static PKey create_from_private_key(std::string_view const &);

 private:
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
