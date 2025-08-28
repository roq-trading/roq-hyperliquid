/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

namespace roq {
namespace hyperliquid {
namespace json {

struct Parser final {
  struct Handler {};

  static bool dispatch(Handler &, std::string_view const &message, std::span<std::byte> const &, TraceInfo const &);
};

}  // namespace json
}  // namespace hyperliquid
}  // namespace roq
