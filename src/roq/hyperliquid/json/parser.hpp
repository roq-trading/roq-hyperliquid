/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/error.hpp"
#include "roq/hyperliquid/json/pong.hpp"
#include "roq/hyperliquid/json/subscription_response.hpp"
//
#include "roq/hyperliquid/json/active_asset_ctx.hpp"
#include "roq/hyperliquid/json/bbo.hpp"
#include "roq/hyperliquid/json/l2_book.hpp"
#include "roq/hyperliquid/json/trades.hpp"
//
#include "roq/hyperliquid/json/spot_meta.hpp"

namespace roq {
namespace hyperliquid {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<json::Pong> const &) = 0;
    virtual void operator()(Trace<json::Error> const &) = 0;
    virtual void operator()(Trace<json::SubscriptionResponse> const &) = 0;
    //
    virtual void operator()(Trace<json::BBO> const &) = 0;
    virtual void operator()(Trace<json::L2Book> const &) = 0;
    virtual void operator()(Trace<json::Trades> const &) = 0;
    virtual void operator()(Trace<json::ActiveAssetCtx> const &) = 0;
    //
    virtual void operator()(Trace<json::SpotMeta> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace hyperliquid
}  // namespace roq
