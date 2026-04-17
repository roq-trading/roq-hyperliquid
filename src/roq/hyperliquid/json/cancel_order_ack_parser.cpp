/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/json/cancel_order_ack_parser.hpp"

#include "roq/logging.hpp"

#include "roq/utils/hash/fnv.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace json {

// === CONSTANTS ===

namespace {
constexpr auto const KEY_STATUS = "status"sv;
constexpr auto const KEY_RESPONSE = "response"sv;
//
constexpr auto const KEY_TYPE = "type"sv;
constexpr auto const KEY_DATA = "data"sv;
//
constexpr auto const KEY_STATUSES = "statuses"sv;
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

bool CancelOrderAckParser::dispatch(
    Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info, bool allow_unknown_event_types) {
  auto result = false;
  auto is_array_of_string = [](auto &value) {
    auto result = true;
    auto handler_2 = utils::overloaded{
        [&](core::json::Null const &) {
          result = false;
          return true;
        },
        [&](bool) {
          result = false;
          return true;
        },
        [&](int64_t) {
          result = false;
          return true;
        },
        [&](double) {
          result = false;
          return true;
        },
        [](std::string_view const &) { return false; },
        [&](core::json::Object const &) {
          result = false;
          return true;
        },
        [&](core::json::Array const &) {
          result = false;
          return true;
        },
    };
    for (auto item : std::get<core::json::Array>(value)) {
      if (std::visit(handler_2, item)) {
        break;
      }
    }
    return result;
  };
  auto data_helper = [&](auto &key, auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_STATUSES):
        if (is_array_of_string(value)) {
          result = true;  // don't dispatch
        } else {
          result = dispatch_helper<CancelOrderAck>(handler, message, buffer_stack, trace_info);
        }
        return true;
    }
    return result;
  };
  auto response_helper = [&](auto &key, auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_TYPE):
        break;
      case utils::hash::FNV::compute(KEY_DATA):
        std::get<core::json::Object>(value).dispatch(data_helper);
        break;
    }
    return result;
  };
  auto helper = [&](auto &key, auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_STATUS):
        break;
      case utils::hash::FNV::compute(KEY_RESPONSE): {
        auto handler_2 = utils::overloaded{
            [](core::json::Null const &) { return false; },
            [](bool) { return false; },
            [](int64_t) { return false; },
            [](double) { return false; },
            [&](std::string_view const &) {
              result = dispatch_helper<GeneralError>(handler, message, buffer_stack, trace_info);
              return true;
            },
            [&](core::json::Object &value) {
              value.dispatch(response_helper);
              return true;
            },
            [](core::json::Array const &) { return false; },
        };
        return std::visit(handler_2, value);
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
