/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/gateway/request.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace gateway {

// === IMPLEMENTATION ===

Request::Request(Shared &shared) : spot_clearing_house_state{*this}, clearing_house_state{*this}, orders{*this}, trades{*this}, shared_{shared} {
}

bool Request::ready() const {
  return spot_clearing_house_state.ready() && clearing_house_state.ready() && orders.ready() && trades.ready();
}

bool Request::can_request(TraceInfo const &trace_info) {
  return shared_.rate_limiter.can_request(trace_info);
}

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
