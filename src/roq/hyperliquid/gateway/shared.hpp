/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/symbols.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/hyperliquid/gateway/api.hpp"
#include "roq/hyperliquid/gateway/settings.hpp"

namespace roq {
namespace hyperliquid {
namespace gateway {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared const &) = delete;

  server::Dispatcher &dispatcher;

  Settings const &settings;
  API const api;

  core::limit::RateLimiter rate_limiter;

  core::Symbols symbols;
  utils::unordered_set<std::string> all_symbols;

  std::vector<MBPUpdate> bids, asks, final_bids, final_asks;
  std::vector<Trade> trades;
  std::vector<Fill> fills;

  // DEX
  struct Dex final {
    std::string name;
    int32_t asset_id_offset = {};
  };
  std::vector<Dex> dex;
};

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
