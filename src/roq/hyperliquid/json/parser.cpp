/* Copyright (c) 2017-2026, Hans Erik Thrane */

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
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(
    Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info, bool allow_unknown_event_types) {
  Message message_2{message, buffer_stack};
  switch (message_2.channel) {
    using enum Channel::type_t;
    case UNDEFINED_INTERNAL:
      break;
    case UNKNOWN_INTERNAL:
      if (allow_unknown_event_types) {
        return false;
      }
      break;
    case PONG:
      dispatch_helper<Pong>(handler, message, buffer_stack, trace_info);
      return true;
    case ERROR:
      dispatch_helper<Error>(handler, message, buffer_stack, trace_info);
      return true;
    case SUBSCRIPTION_RESPONSE:
      dispatch_helper<SubscriptionResponse>(handler, message, buffer_stack, trace_info);
      return true;
    case BBO:
      dispatch_helper<json::BBO>(handler, message, buffer_stack, trace_info);
      return true;
    case L2_BOOK:
      dispatch_helper<L2Book>(handler, message, buffer_stack, trace_info);
      return true;
    case TRADES:
      dispatch_helper<Trades>(handler, message, buffer_stack, trace_info);
      return true;
    case ACTIVE_ASSET_CTX:
      dispatch_helper<ActiveAssetCtx>(handler, message, buffer_stack, trace_info);
      return true;
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace hyperliquid
}  // namespace roq
