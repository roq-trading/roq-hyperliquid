/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/hyperliquid/json/parser.hpp"

#include "roq/logging.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/core/json/array_parser.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace json {

// === HELPERS ===

namespace {}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(Handler &handler, std::string_view const &message, std::span<std::byte> const &buffer, TraceInfo const &trace_info) {
  // Message message_{message, buffer};
  return false;
}

}  // namespace json
}  // namespace hyperliquid
}  // namespace roq
