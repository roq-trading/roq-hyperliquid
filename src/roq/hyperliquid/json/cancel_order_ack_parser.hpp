/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/hyperliquid/json/cancel_order_ack.hpp"
#include "roq/hyperliquid/json/general_error.hpp"

namespace roq {
namespace hyperliquid {
namespace json {

struct CancelOrderAckParser final {
  template <typename ErrorHandler, typename SuccessHandler>
  static bool dispatch(
      std::string_view const &message,
      core::json::BufferStack &buffer_stack,
      TraceInfo const &trace_info,
      bool allow_unknown_event_types,
      ErrorHandler error_handler,
      SuccessHandler success_handler) {
    struct MyHandler final : public Handler {
      MyHandler(ErrorHandler error_handler, SuccessHandler success_handler) : error_handler_{error_handler}, success_handler_{success_handler} {}

     protected:
      void operator()(Trace<json::GeneralError> const &event) {
        auto &[trace_info, general_error] = event;
        error_handler_(general_error.response);
      }
      void operator()(Trace<json::CancelOrderAck> const &event) { success_handler_(event); }

     private:
      ErrorHandler error_handler_;
      SuccessHandler success_handler_;
    } handler{error_handler, success_handler};
    return dispatch(handler, message, buffer_stack, trace_info, allow_unknown_event_types);
  }

 protected:
  struct Handler {
    virtual void operator()(Trace<json::GeneralError> const &) = 0;
    virtual void operator()(Trace<json::CancelOrderAck> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace hyperliquid
}  // namespace roq
