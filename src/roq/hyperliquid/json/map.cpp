/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/hyperliquid/json/map.hpp"

// using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// hyperliquid::json => roq

// hyperliquid::json::Side ==> roq::Side

template <>
template <>
constexpr Helper<hyperliquid::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum hyperliquid::json::Side::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Side::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Side::UNDEFINED;
    case BID:
      return roq::Side::BUY;
    case ASK:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{hyperliquid::json::Side{hyperliquid::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{hyperliquid::json::Side{hyperliquid::json::Side::BID}} == roq::Side::BUY);
static_assert(Helper{hyperliquid::json::Side{hyperliquid::json::Side::ASK}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<hyperliquid::json::Side>::helper() const {
  return Helper{args_};
}

}  // namespace roq
