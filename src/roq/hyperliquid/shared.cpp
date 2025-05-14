/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/hyperliquid/shared.hpp"

#include <algorithm>
#include <cctype>

#include <magic_enum/magic_enum.hpp>

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : dispatcher{dispatcher}, settings{settings}, api{API::create(settings)}, rate_limiter{settings.request.limit, settings.request.limit_interval},
      symbols{settings.ws.max_subscriptions_per_stream} {
}

}  // namespace hyperliquid
}  // namespace roq
