/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/protocol/json/error.hpp"
#include "roq/hyperliquid/protocol/json/pong.hpp"
#include "roq/hyperliquid/protocol/json/subscription_response.hpp"
//
#include "roq/hyperliquid/protocol/json/active_asset_ctx.hpp"
#include "roq/hyperliquid/protocol/json/bbo.hpp"
#include "roq/hyperliquid/protocol/json/l2_book.hpp"
#include "roq/hyperliquid/protocol/json/trades.hpp"
//
#include "roq/hyperliquid/protocol/json/spot_meta.hpp"
//
#include "roq/hyperliquid/protocol/json/notification.hpp"
#include "roq/hyperliquid/protocol/json/order_updates.hpp"
#include "roq/hyperliquid/protocol/json/user.hpp"
#include "roq/hyperliquid/protocol/json/user_fills.hpp"
#include "roq/hyperliquid/protocol/json/user_fundings.hpp"

namespace roq {
namespace hyperliquid {
namespace protocol {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<protocol::json::Pong> const &) = 0;
    virtual void operator()(Trace<protocol::json::Error> const &) = 0;
    virtual void operator()(Trace<protocol::json::SubscriptionResponse> const &) = 0;
    //
    virtual void operator()(Trace<protocol::json::BBO> const &) = 0;
    virtual void operator()(Trace<protocol::json::L2Book> const &) = 0;
    virtual void operator()(Trace<protocol::json::Trades> const &) = 0;
    virtual void operator()(Trace<protocol::json::ActiveAssetCtx> const &) = 0;
    //
    virtual void operator()(Trace<protocol::json::SpotMeta> const &) = 0;
    //
    virtual void operator()(Trace<protocol::json::User> const &) = 0;
    virtual void operator()(Trace<protocol::json::UserFundings> const &) = 0;
    virtual void operator()(Trace<protocol::json::UserFills> const &) = 0;
    virtual void operator()(Trace<protocol::json::OrderUpdates> const &) = 0;
    virtual void operator()(Trace<protocol::json::Notification> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace protocol
}  // namespace hyperliquid
}  // namespace roq
