/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/hyperliquid/json/map.hpp"

// using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

/*
// hyperliquid::json => roq

// hyperliquid::json::EventType ==> roq::UpdateType

template <>
template <>
constexpr Helper<hyperliquid::json::EventType>::operator std::optional<roq::UpdateType>() const {
  switch (std::get<0>(args_)) {
    using enum hyperliquid::json::EventType::type_t;
    case UNDEFINED_INTERNAL:
      return UpdateType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return UpdateType::UNDEFINED;
    case ERROR:
      return UpdateType::UNDEFINED;
    case SNAPSHOT:
      return UpdateType::SNAPSHOT;
    case DELTA:
      return UpdateType::INCREMENTAL;
    case COMMAND_RESP:
      return UpdateType::UNDEFINED;
  }
  return {};
}

static_assert(Helper{hyperliquid::json::EventType{hyperliquid::json::EventType::UNDEFINED_INTERNAL}} == roq::UpdateType::UNDEFINED);
static_assert(Helper{hyperliquid::json::EventType{hyperliquid::json::EventType::SNAPSHOT}} == roq::UpdateType::SNAPSHOT);
static_assert(Helper{hyperliquid::json::EventType{hyperliquid::json::EventType::DELTA}} == roq::UpdateType::INCREMENTAL);
static_assert(Helper{hyperliquid::json::EventType{hyperliquid::json::EventType::COMMAND_RESP}} == roq::UpdateType::UNDEFINED);

template <>
template <>
std::optional<roq::UpdateType> Map<hyperliquid::json::EventType>::helper() const {
  return Helper{args_};
}

// hyperliquid::json::OptionsType ==> roq::OptionType

template <>
template <>
constexpr Helper<hyperliquid::json::OptionsType>::operator std::optional<roq::OptionType>() const {
  switch (std::get<0>(args_)) {
    using enum hyperliquid::json::OptionsType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OptionType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OptionType::UNDEFINED;
    case CALL:
      return roq::OptionType::CALL;
    case PUT:
      return roq::OptionType::PUT;
  }
  return {};
}

static_assert(Helper{hyperliquid::json::OptionsType{hyperliquid::json::OptionsType::UNDEFINED_INTERNAL}} == roq::OptionType::UNDEFINED);
static_assert(Helper{hyperliquid::json::OptionsType{hyperliquid::json::OptionsType::CALL}} == roq::OptionType::CALL);
static_assert(Helper{hyperliquid::json::OptionsType{hyperliquid::json::OptionsType::PUT}} == roq::OptionType::PUT);

template <>
template <>
std::optional<roq::OptionType> Map<hyperliquid::json::OptionsType>::helper() const {
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
    case CREATED:
      return roq::OrderStatus::WORKING;
    case NEW:
      return roq::OrderStatus::WORKING;
    case REJECTED:
      return roq::OrderStatus::REJECTED;
    case PARTIALLY_FILLED:
      return roq::OrderStatus::WORKING;
    case PARTIALLY_FILLED_CANCELED:
      return roq::OrderStatus::CANCELED;
    case FILLED:
      return roq::OrderStatus::COMPLETED;
    case CANCELLED:
      return roq::OrderStatus::CANCELED;
    case UNTRIGGERED:
      return roq::OrderStatus::UNDEFINED;
    case TRIGGERED:
      return roq::OrderStatus::UNDEFINED;
    case DEACTIVATED:
      return roq::OrderStatus::UNDEFINED;
    case ACTIVE:
      return roq::OrderStatus::UNDEFINED;
  }
  return {};
}

static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::CREATED}} == roq::OrderStatus::WORKING);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::NEW}} == roq::OrderStatus::WORKING);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::REJECTED}} == roq::OrderStatus::REJECTED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::PARTIALLY_FILLED}} == roq::OrderStatus::WORKING);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::PARTIALLY_FILLED_CANCELED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::CANCELLED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::UNTRIGGERED}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::TRIGGERED}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::DEACTIVATED}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{hyperliquid::json::OrderStatus{hyperliquid::json::OrderStatus::ACTIVE}} == roq::OrderStatus::UNDEFINED);

template <>
template <>
std::optional<roq::OrderStatus> Map<hyperliquid::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// hyperliquid::json::OrderType ==> roq::OrderType

template <>
template <>
constexpr Helper<hyperliquid::json::OrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum hyperliquid::json::OrderType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN:
      return roq::OrderType::UNDEFINED;
    case MARKET:
      return roq::OrderType::MARKET;
    case LIMIT:
      return roq::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{hyperliquid::json::OrderType{hyperliquid::json::OrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{hyperliquid::json::OrderType{hyperliquid::json::OrderType::MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{hyperliquid::json::OrderType{hyperliquid::json::OrderType::LIMIT}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<hyperliquid::json::OrderType>::helper() const {
  return Helper{args_};
}

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
    case NONE:
      return roq::Side::UNDEFINED;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{hyperliquid::json::Side{hyperliquid::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{hyperliquid::json::Side{hyperliquid::json::Side::NONE}} == roq::Side::UNDEFINED);
static_assert(Helper{hyperliquid::json::Side{hyperliquid::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{hyperliquid::json::Side{hyperliquid::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<hyperliquid::json::Side>::helper() const {
  return Helper{args_};
}

// hyperliquid::json::Status ==> roq::TradingStatus

template <>
template <>
constexpr Helper<hyperliquid::json::Status>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum hyperliquid::json::Status::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
    case PRE_LAUNCH:
      return roq::TradingStatus::UNDEFINED;
    case TRADING:
      return roq::TradingStatus::OPEN;
    case SETTLING:
      return roq::TradingStatus::UNDEFINED;
    case DELIVERING:
      return roq::TradingStatus::UNDEFINED;
    case CLOSED:
      return roq::TradingStatus::CLOSE;
  }
  return {};
}

static_assert(Helper{hyperliquid::json::Status{hyperliquid::json::Status::UNDEFINED_INTERNAL}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{hyperliquid::json::Status{hyperliquid::json::Status::PRE_LAUNCH}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{hyperliquid::json::Status{hyperliquid::json::Status::TRADING}} == roq::TradingStatus::OPEN);
static_assert(Helper{hyperliquid::json::Status{hyperliquid::json::Status::SETTLING}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{hyperliquid::json::Status{hyperliquid::json::Status::DELIVERING}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{hyperliquid::json::Status{hyperliquid::json::Status::CLOSED}} == roq::TradingStatus::CLOSE);

template <>
template <>
std::optional<roq::TradingStatus> Map<hyperliquid::json::Status>::helper() const {
  return Helper{args_};
}

// hyperliquid::json::TimeInForce ==> roq::TimeInForce

template <>
template <>
constexpr Helper<hyperliquid::json::TimeInForce>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum hyperliquid::json::TimeInForce::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case GTC:
      return roq::TimeInForce::GTC;
    case FOK:
      return roq::TimeInForce::FOK;
    case IOC:
      return roq::TimeInForce::IOC;
  }
  return {};
}

static_assert(Helper{hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::GTC}} == roq::TimeInForce::GTC);
static_assert(Helper{hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::IOC}} == roq::TimeInForce::IOC);

template <>
template <>
std::optional<roq::TimeInForce> Map<hyperliquid::json::TimeInForce>::helper() const {
  return Helper{args_};
}

// hyperliquid::json::{ ContractType, OptionsType } ==> roq::SecurityType

template <>
template <>
constexpr Helper<hyperliquid::json::ContractType, hyperliquid::json::OptionsType>::operator std::optional<roq::SecurityType>() const {
  switch (std::get<1>(args_)) {
    using enum hyperliquid::json::OptionsType::type_t;
    case UNDEFINED_INTERNAL:
    case UNKNOWN_INTERNAL:
      break;
    case CALL:
    case PUT:
      return SecurityType::OPTION;
  }
  switch (std::get<0>(args_)) {
    using enum hyperliquid::json::ContractType::type_t;
    case UNDEFINED_INTERNAL:
    case UNKNOWN_INTERNAL:
      break;
    case INVERSE_PERPETUAL:
    case LINEAR_PERPETUAL:
      return SecurityType::SWAP;
    case LINEAR_FUTURES:
    case INVERSE_FUTURES:
      return SecurityType::FUTURES;
  }
  return SecurityType::SPOT;
}

static_assert(
    Helper{hyperliquid::json::ContractType{hyperliquid::json::ContractType::UNDEFINED_INTERNAL},
hyperliquid::json::OptionsType{hyperliquid::json::OptionsType::UNDEFINED_INTERNAL}} == roq::SecurityType::SPOT); static_assert(
    Helper{hyperliquid::json::ContractType{hyperliquid::json::ContractType::UNDEFINED_INTERNAL},
hyperliquid::json::OptionsType{hyperliquid::json::OptionsType::CALL}} == roq::SecurityType::OPTION); static_assert(
    Helper{hyperliquid::json::ContractType{hyperliquid::json::ContractType::UNDEFINED_INTERNAL},
hyperliquid::json::OptionsType{hyperliquid::json::OptionsType::PUT}} == roq::SecurityType::OPTION); static_assert(
    Helper{hyperliquid::json::ContractType{hyperliquid::json::ContractType::INVERSE_PERPETUAL},
hyperliquid::json::OptionsType{hyperliquid::json::OptionsType::UNDEFINED_INTERNAL}} == roq::SecurityType::SWAP); static_assert(
    Helper{hyperliquid::json::ContractType{hyperliquid::json::ContractType::LINEAR_PERPETUAL},
hyperliquid::json::OptionsType{hyperliquid::json::OptionsType::UNDEFINED_INTERNAL}} == roq::SecurityType::SWAP); static_assert(
    Helper{hyperliquid::json::ContractType{hyperliquid::json::ContractType::LINEAR_FUTURES},
hyperliquid::json::OptionsType{hyperliquid::json::OptionsType::UNDEFINED_INTERNAL}} == roq::SecurityType::FUTURES); static_assert(
    Helper{hyperliquid::json::ContractType{hyperliquid::json::ContractType::INVERSE_FUTURES},
hyperliquid::json::OptionsType{hyperliquid::json::OptionsType::UNDEFINED_INTERNAL}} == roq::SecurityType::FUTURES);

template <>
template <>
std::optional<roq::SecurityType> Map<hyperliquid::json::ContractType, hyperliquid::json::OptionsType>::helper() const {
  return Helper{args_};
}

// roq ==>

// roq::OrderType ==> hyperliquid::json::OrderType

template <>
template <>
constexpr Helper<roq::OrderType>::operator std::optional<hyperliquid::json::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return hyperliquid::json::OrderType::UNDEFINED_INTERNAL;
    case MARKET:
      return hyperliquid::json::OrderType::MARKET;
    case LIMIT:
      return hyperliquid::json::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{roq::OrderType::UNDEFINED} == hyperliquid::json::OrderType{hyperliquid::json::OrderType::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderType::MARKET} == hyperliquid::json::OrderType{hyperliquid::json::OrderType::MARKET});
static_assert(Helper{roq::OrderType::LIMIT} == hyperliquid::json::OrderType{hyperliquid::json::OrderType::LIMIT});

template <>
template <>
std::optional<hyperliquid::json::OrderType> Map<roq::OrderType>::helper() const {
  return Helper{args_};
}

// roq::Side ==> hyperliquid::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<hyperliquid::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return hyperliquid::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return hyperliquid::json::Side::BUY;
    case SELL:
      return hyperliquid::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == hyperliquid::json::Side{hyperliquid::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == hyperliquid::json::Side{hyperliquid::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == hyperliquid::json::Side{hyperliquid::json::Side::SELL});

template <>
template <>
std::optional<hyperliquid::json::Side> Map<roq::Side>::helper() const {
  return Helper{args_};
}

// roq::TimeInForce ==> hyperliquid::json::TimeInForce

template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<hyperliquid::json::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
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
      return hyperliquid::json::TimeInForce::FOK;
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
static_assert(Helper{roq::TimeInForce::GTC} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::GTC});
static_assert(Helper{roq::TimeInForce::IOC} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::IOC});
static_assert(Helper{roq::TimeInForce::FOK} == hyperliquid::json::TimeInForce{hyperliquid::json::TimeInForce::FOK});

template <>
template <>
std::optional<hyperliquid::json::TimeInForce> Map<roq::TimeInForce>::helper() const {
  return Helper{args_};
}
*/
}  // namespace roq
