/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/hyperliquid/gateway/config.hpp"
#include "roq/hyperliquid/gateway/settings.hpp"

#include "roq/hyperliquid/tools/crypto.hpp"

namespace roq {
namespace hyperliquid {
namespace gateway {

struct Account final {
  Account(Settings const &, Config const &, std::string_view const &name);

  Account(Account const &) = delete;

  std::string_view get_key() const { return crypto_.get_key(); }

  std::string sign_l1_action(
      std::string_view const &action, std::span<std::byte> const &packed, std::chrono::milliseconds now_utc, std::chrono::milliseconds expires_after_utc);

  std::string const name;

 private:
  tools::Crypto crypto_;
};

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
