/* Copyright (c) 2017-2026, Hans Erik Thrane */

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

// hyperliquid::json::OrderStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<hyperliquid::json::OrderStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum hyperliquid::json::OrderStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case OPEN:
      return roq::OrderStatus::WORKING;
    case CANCELED:
      return roq::OrderStatus::CANCELED;
  }
  return {};
}

static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::OPEN}} == roq::OrderStatus::WORKING);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::CANCELED}} == roq::OrderStatus::CANCELED);

template <>
template <>
std::optional<roq::OrderStatus> Map<hyperliquid::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// hyperliquid::json => roq

// roq::Side => hyperliquid::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<hyperliquid::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum Side;
    case UNDEFINED:
      return hyperliquid::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return hyperliquid::json::Side::BID;
    case SELL:
      return hyperliquid::json::Side::ASK;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == hyperliquid::json::Side{hyperliquid::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == hyperliquid::json::Side{hyperliquid::json::Side::BID});
static_assert(Helper{roq::Side::SELL} == hyperliquid::json::Side{hyperliquid::json::Side::ASK});

template <>
template <>
std::optional<hyperliquid::json::Side> Map<roq::Side>::helper() const {
  return Helper{args_};
}

}  // namespace roq
