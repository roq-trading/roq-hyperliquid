/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/hyperliquid/json/contract_type.hpp"
#include "roq/hyperliquid/json/event_type.hpp"
#include "roq/hyperliquid/json/options_type.hpp"
#include "roq/hyperliquid/json/order_status.hpp"
#include "roq/hyperliquid/json/order_type.hpp"
#include "roq/hyperliquid/json/side.hpp"
#include "roq/hyperliquid/json/status.hpp"
#include "roq/hyperliquid/json/time_in_force.hpp"

#include "roq/option_type.hpp"
#include "roq/order_status.hpp"
#include "roq/order_type.hpp"
#include "roq/security_type.hpp"
#include "roq/side.hpp"
#include "roq/time_in_force.hpp"
#include "roq/trading_status.hpp"
#include "roq/update_type.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<UpdateType> Map<hyperliquid::json::EventType>::helper() const;

template <>
template <>
std::optional<OptionType> Map<hyperliquid::json::OptionsType>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<hyperliquid::json::OrderStatus>::helper() const;

template <>
template <>
std::optional<OrderType> Map<hyperliquid::json::OrderType>::helper() const;

template <>
template <>
std::optional<Side> Map<hyperliquid::json::Side>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<hyperliquid::json::Status>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<hyperliquid::json::TimeInForce>::helper() const;

// ===

template <>
template <>
std::optional<SecurityType> Map<hyperliquid::json::ContractType, hyperliquid::json::OptionsType>::helper() const;

// ===

template <>
template <>
std::optional<hyperliquid::json::OrderType> Map<OrderType>::helper() const;

template <>
template <>
std::optional<hyperliquid::json::Side> Map<Side>::helper() const;

template <>
template <>
std::optional<hyperliquid::json::TimeInForce> Map<TimeInForce>::helper() const;

}  // namespace roq
