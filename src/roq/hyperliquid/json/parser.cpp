/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/hyperliquid/json/parser.hpp"

#include "roq/logging.hpp"

#include "roq/hyperliquid/json/message.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace json {

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer, auto &trace_info) {
  T obj{message, buffer};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(Handler &handler, std::string_view const &message, std::span<std::byte> const &buffer, TraceInfo const &trace_info) {
  Message message_{message, buffer};
  switch (message_.channel) {
    using enum Channel::type_t;
    case UNDEFINED_INTERNAL:
    case UNKNOWN_INTERNAL:
      log::fatal("Unexpected"sv);
    case PONG:
      dispatch_helper<Pong>(handler, message, buffer, trace_info);
      return true;
    case ERROR:
      dispatch_helper<Error>(handler, message, buffer, trace_info);
      return true;
    case SUBSCRIPTION_RESPONSE:
      dispatch_helper<SubscriptionResponse>(handler, message, buffer, trace_info);
      return true;
    case BBO:
      dispatch_helper<json::BBO>(handler, message, buffer, trace_info);
      return true;
    case TRADES:
      dispatch_helper<Trades>(handler, message, buffer, trace_info);
      return true;
  }
  return false;
}

}  // namespace json
}  // namespace hyperliquid
}  // namespace roq
