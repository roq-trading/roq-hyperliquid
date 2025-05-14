/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/hyperliquid/json/error.hpp"
#include "roq/hyperliquid/json/ping.hpp"
#include "roq/hyperliquid/json/subscribe.hpp"

// public
#include "roq/hyperliquid/json/order_book.hpp"
#include "roq/hyperliquid/json/public_trade.hpp"
#include "roq/hyperliquid/json/tickers.hpp"

// private
#include "roq/hyperliquid/json/auth.hpp"
#include "roq/hyperliquid/json/execution_2.hpp"
#include "roq/hyperliquid/json/order.hpp"
#include "roq/hyperliquid/json/position.hpp"
#include "roq/hyperliquid/json/wallet.hpp"

namespace roq {
namespace hyperliquid {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<json::Ping> const &) = 0;
    // response
    virtual void operator()(Trace<json::Auth> const &) = 0;
    virtual void operator()(Trace<json::Subscribe> const &) = 0;
    virtual void operator()(Trace<json::Error> const &) = 0;
    // public stream
    virtual void operator()(Trace<json::OrderBook> const &, size_t depth) = 0;
    virtual void operator()(Trace<json::PublicTrade> const &) = 0;
    virtual void operator()(Trace<json::Tickers> const &) = 0;
    // private stream
    virtual void operator()(Trace<json::Wallet> const &) = 0;
    virtual void operator()(Trace<json::Position> const &) = 0;
    virtual void operator()(Trace<json::Order> const &) = 0;
    virtual void operator()(Trace<json::Execution2> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, std::span<std::byte> const &, TraceInfo const &);
};

}  // namespace json
}  // namespace hyperliquid
}  // namespace roq
