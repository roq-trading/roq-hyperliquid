/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/hyperliquid/json/parser.hpp"

#include "roq/logging.hpp"

#include "roq/utils/enum.hpp"

#include "roq/core/json/array_parser.hpp"

#include "roq/hyperliquid/json/message.hpp"
#include "roq/hyperliquid/json/utils.hpp"

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

void parse_l2book_data(auto &result, auto &value, auto &buffer_stack) {
  enum class Key {
    COIN,
    TIME,
    LEVELS,
  };
  for (auto [key, value] : std::get<core::json::Object>(value)) {
    auto key_2 = utils::parse_enum<Key>(key);
    switch (key_2) {
      case Key::COIN:
        update(result.coin, value);
        break;
      case Key::TIME:
        update(result.time, value);
        break;
      case Key::LEVELS: {
        size_t offset = 0;
        for (auto value_2 : std::get<core::json::Array>(value)) {
          switch (++offset) {
            case 1:
              result.bids = core::json::ArrayParser<decltype(result.bids), core::json::Array>::parse(buffer_stack, std::get<core::json::Array>(value_2));
              break;
            case 2:
              result.asks = core::json::ArrayParser<decltype(result.asks), core::json::Array>::parse(buffer_stack, std::get<core::json::Array>(value_2));
              break;
            default:
              log::fatal("Unexpected"sv);
          }
        }
        break;
      }
    }
  }
}

auto parse_l2book(auto &value, auto &buffer_stack) {
  L2Book result;
  enum class Key {
    CHANNEL,
    DATA,
  };
  for (auto [key, value] : std::get<core::json::Object>(value)) {
    auto key_2 = utils::parse_enum<Key>(key);
    switch (key_2) {
      case Key::CHANNEL:
        result.channel = Channel::L2_BOOK;
        break;
      case Key::DATA:
        parse_l2book_data(result.data, value, buffer_stack);
        break;
    }
  }
  return result;
}

// levels are double-nested
void dispatch_l2book(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  core::json::Parser parser{message};
  auto value = parser.root();
  auto obj = parse_l2book(value, buffer_stack);
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info) {
  Message message_{message, buffer_stack};
  switch (message_.channel) {
    using enum Channel::type_t;
    case UNDEFINED_INTERNAL:
    case UNKNOWN_INTERNAL:
      log::fatal("Unexpected"sv);
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
      dispatch_l2book(handler, message, buffer_stack, trace_info);
      return true;
    case TRADES:
      dispatch_helper<Trades>(handler, message, buffer_stack, trace_info);
      return true;
    case ACTIVE_ASSET_CTX:
      dispatch_helper<ActiveAssetCtx>(handler, message, buffer_stack, trace_info);
      return true;
  }
  return false;
}

}  // namespace json
}  // namespace hyperliquid
}  // namespace roq
