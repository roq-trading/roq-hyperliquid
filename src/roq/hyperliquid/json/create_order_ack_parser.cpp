/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/json/create_order_ack_parser.hpp"

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
//
constexpr auto KEY_ERROR = "error"sv;
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

bool CreateOrderAckParser::dispatch(
    Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info, bool allow_unknown_event_types) {
  auto result = false;
  /*
  auto status_helper = [&](auto &key, [[maybe_unused]] auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_ERROR):
        break;
    }
    return false;
  };
  */
  auto data_helper = [&](auto &key, auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_STATUSES):
        if (true) {
          result = dispatch_helper<CreateOrderAck>(handler, message, buffer_stack, trace_info);
          return true;
        } else {
          for (auto item : std::get<core::json::Array>(value)) {
            // XXX could be status (dict) or string
            auto handler_2 = utils::overloaded{
                [](core::json::Null const &) { return false; },
                [](bool) { return false; },
                [](int64_t) { return false; },
                [](double) { return false; },
                [](std::string_view const &) {
                  // generic error
                  return true;
                },
                [&](core::json::Object &) {
                  result = dispatch_helper<CreateOrderAck>(handler, message, buffer_stack, trace_info);
                  // value.dispatch(status_helper);
                  return true;
                },
                [](core::json::Array const &) { return false; },
            };
            return std::visit(handler_2, value);
          }
        }
        break;
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
