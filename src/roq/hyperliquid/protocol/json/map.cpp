/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/protocol/json/map.hpp"

// using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// hyperliquid::json => roq

// hyperliquid::protocol::json::Side ==> roq::Side

template <>
template <>
constexpr Helper<hyperliquid::protocol::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum hyperliquid::protocol::json::Side::type_t;
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

static_assert(Helper{hyperliquid::protocol::json::Side{hyperliquid::protocol::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{hyperliquid::protocol::json::Side{hyperliquid::protocol::json::Side::BID}} == roq::Side::BUY);
static_assert(Helper{hyperliquid::protocol::json::Side{hyperliquid::protocol::json::Side::ASK}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<hyperliquid::protocol::json::Side>::helper() const {
  return Helper{args_};
}

// hyperliquid::protocol::json::OrderStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<hyperliquid::protocol::json::OrderStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum hyperliquid::protocol::json::OrderStatus::type_t;
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
    case BAD_ALO_PX_REJECTED:
      return roq::OrderStatus::REJECTED;
  }
  return {};
}

static_assert(Helper{hyperliquid::protocol::json::OrderStatus{hyperliquid::protocol::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{hyperliquid::protocol::json::OrderStatus{hyperliquid::protocol::json::OrderStatus::OPEN}} == roq::OrderStatus::WORKING);
static_assert(Helper{hyperliquid::protocol::json::OrderStatus{hyperliquid::protocol::json::OrderStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{hyperliquid::protocol::json::OrderStatus{hyperliquid::protocol::json::OrderStatus::CANCELED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{hyperliquid::protocol::json::OrderStatus{hyperliquid::protocol::json::OrderStatus::MIN_TRADE_NTL_REJECTED}} == roq::OrderStatus::REJECTED);
static_assert(Helper{hyperliquid::protocol::json::OrderStatus{hyperliquid::protocol::json::OrderStatus::IOC_CANCEL_REJECTED}} == roq::OrderStatus::REJECTED);
static_assert(Helper{hyperliquid::protocol::json::OrderStatus{hyperliquid::protocol::json::OrderStatus::PERP_MARGIN_REJECTED}} == roq::OrderStatus::REJECTED);
static_assert(Helper{hyperliquid::protocol::json::OrderStatus{hyperliquid::protocol::json::OrderStatus::BAD_ALO_PX_REJECTED}} == roq::OrderStatus::REJECTED);

template <>
template <>
std::optional<roq::OrderStatus> Map<hyperliquid::protocol::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// hyperliquid::json => roq

// roq::Side => hyperliquid::protocol::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<hyperliquid::protocol::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum Side;
    case UNDEFINED:
      return hyperliquid::protocol::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return hyperliquid::protocol::json::Side::BID;
    case SELL:
      return hyperliquid::protocol::json::Side::ASK;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == hyperliquid::protocol::json::Side{hyperliquid::protocol::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == hyperliquid::protocol::json::Side{hyperliquid::protocol::json::Side::BID});
static_assert(Helper{roq::Side::SELL} == hyperliquid::protocol::json::Side{hyperliquid::protocol::json::Side::ASK});

template <>
template <>
std::optional<hyperliquid::protocol::json::Side> Map<roq::Side>::helper() const {
  return Helper{args_};
}

// roq::TimeInForce => hyperliquid::protocol::json::TimeInForce

template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<hyperliquid::protocol::json::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum TimeInForce;
    case UNDEFINED:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFD:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTC:
      return hyperliquid::protocol::json::TimeInForce::GTC;
    case OPG:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case IOC:
      return hyperliquid::protocol::json::TimeInForce::IOC;
    case FOK:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTX:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTD:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_THE_CLOSE:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_THROUGH_CROSSING:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_CROSSING:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_FOR_TIME:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFA:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFM:
      return hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::TimeInForce::UNDEFINED} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFD} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTC} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::GTC});
static_assert(Helper{roq::TimeInForce::OPG} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::IOC} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::IOC});
static_assert(Helper{roq::TimeInForce::FOK} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTX} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTD} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_THE_CLOSE} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_CROSSING} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::GOOD_FOR_TIME} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFA} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFM} == hyperliquid::protocol::json::TimeInForce{hyperliquid::protocol::json::TimeInForce::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<hyperliquid::protocol::json::TimeInForce> Map<roq::TimeInForce>::helper() const {
  return Helper{args_};
}

}  // namespace roq
