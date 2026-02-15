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
    case FILLED:
      return roq::OrderStatus::COMPLETED;
    case CANCELED:
      return roq::OrderStatus::CANCELED;
    case MIN_TRADE_NTL_REJECTED:
      return roq::OrderStatus::REJECTED;
    case IOC_CANCEL_REJECTED:
      return roq::OrderStatus::REJECTED;
    case PERP_MARGIN_REJECTED:
      return roq::OrderStatus::REJECTED;
  }
  return {};
}

static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::OPEN}} == roq::OrderStatus::WORKING);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::CANCELED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::MIN_TRADE_NTL_REJECTED}} == roq::OrderStatus::REJECTED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::IOC_CANCEL_REJECTED}} == roq::OrderStatus::REJECTED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::PERP_MARGIN_REJECTED}} == roq::OrderStatus::REJECTED);

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

// roq::TimeInForce => hyperliquid::json::TimeInForce

template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<hyperliquid::json::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum TimeInForce;
    case UNDEFINED:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFD:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTC:
      return hyperliquid::json::TimeInForce::GTC;
    case OPG:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case IOC:
      return hyperliquid::json::TimeInForce::IOC;
    case FOK:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTX:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTD:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_THE_CLOSE:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_THROUGH_CROSSING:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_CROSSING:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_FOR_TIME:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFA:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFM:
      return hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::TimeInForce::UNDEFINED} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFD} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTC} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::GTC});
static_assert(Helper{roq::TimeInForce::OPG} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::IOC} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::IOC});
static_assert(Helper{roq::TimeInForce::FOK} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTX} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTD} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_THE_CLOSE} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_CROSSING} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_FOR_TIME} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFA} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFM} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<hyperliquid::json::TimeInForce> Map<roq::TimeInForce>::helper() const {
  return Helper{args_};
}

}  // namespace roq
