/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/json/parser.hpp"

#include "roq/logging.hpp"

#include "roq/utils/hash/fnv.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace json {

// === CONSTANTS ===

namespace {
constexpr auto const KEY_CHANNEL = "channel"sv;
constexpr auto const KEY_DATA = "data"sv;
constexpr auto const KEY_RESPONSE = "response"sv;
constexpr auto const KEY_PAYLOAD = "payload"sv;
constexpr auto const KEY_TYPE = "type"sv;
}  // namespace

// === HELPERS ===

namespace {
template <typename T>
bool dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
  return true;
}
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(
    Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info, bool allow_unknown_event_types) {
  auto result = false;
  auto helper_post_response_payload = [&](auto &key, auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_TYPE): {
        PayloadType type{value};
        switch (type) {
          using enum PayloadType::type_t;
          case UNDEFINED_INTERNAL:
            break;
          case UNKNOWN_INTERNAL:
            if (allow_unknown_event_types) {
              return false;
            }
            break;
          case SPOT_META:
            result = dispatch_helper<SpotMeta>(handler, message, buffer_stack, trace_info);
            return true;
        }
        break;
      }
    }
    return result;
  };
  auto helper_post_response = [&](auto &key, auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_PAYLOAD): {
        std::get<core::json::Object>(value).dispatch(helper_post_response_payload);
        break;
      }
    }
    return result;
  };
  auto helper_post = [&](auto &key, auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_RESPONSE): {
        std::get<core::json::Object>(value).dispatch(helper_post_response);
        return true;
      }
    }
    return result;
  };
  Channel channel;
  auto helper = [&](auto &key, auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_CHANNEL): {
        channel = Channel{value};
        switch (channel) {
          using enum Channel::type_t;
          case UNDEFINED_INTERNAL:
            break;
          case UNKNOWN_INTERNAL:
            if (allow_unknown_event_types) {
              return false;
            }
            break;
          case PONG:
            result = dispatch_helper<Pong>(handler, message, buffer_stack, trace_info);
            return true;
          case ERROR:
            result = dispatch_helper<Error>(handler, message, buffer_stack, trace_info);
            return true;
          case POST:
            // wait for data
            break;
          case SUBSCRIPTION_RESPONSE:
            result = dispatch_helper<SubscriptionResponse>(handler, message, buffer_stack, trace_info);
            return true;
          case BBO:
            result = dispatch_helper<json::BBO>(handler, message, buffer_stack, trace_info);
            return true;
          case L2_BOOK:
            result = dispatch_helper<L2Book>(handler, message, buffer_stack, trace_info);
            return true;
          case TRADES:
            result = dispatch_helper<Trades>(handler, message, buffer_stack, trace_info);
            return true;
          case ACTIVE_ASSET_CTX:
            result = dispatch_helper<ActiveAssetCtx>(handler, message, buffer_stack, trace_info);
            return true;
          case USER:
            result = dispatch_helper<User>(handler, message, buffer_stack, trace_info);
            return true;
          case USER_FUNDINGS:
            result = dispatch_helper<UserFundings>(handler, message, buffer_stack, trace_info);
            return true;
          case USER_FILLS:
            result = dispatch_helper<UserFills>(handler, message, buffer_stack, trace_info);
            return true;
          case ORDER_UPDATES:
            result = dispatch_helper<OrderUpdates>(handler, message, buffer_stack, trace_info);
            return true;
        }
        break;
      }
      case utils::hash::FNV::compute(KEY_DATA): {
        if (channel == Channel::POST) {
          std::get<core::json::Object>(value).dispatch(helper_post);
        }
        break;
      }
    }
    return result;
  };
  core::json::Parser::dispatch<core::json::Object>(helper, message);
  if (result || allow_unknown_event_types) {
    return result;
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace hyperliquid
}  // namespace roq
