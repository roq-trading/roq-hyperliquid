/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/json/utils.hpp"

#include "roq/hyperliquid/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace json {

std::string_view get_client_order_id(std::string_view const &cloid) {
  if (cloid.starts_with("0x"sv)) {
    return cloid.substr(2);
  }
  return cloid;
}

}  // namespace json
}  // namespace hyperliquid
}  // namespace roq
