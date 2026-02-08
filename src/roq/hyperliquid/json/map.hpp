/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/map.hpp"

#include "roq/order_status.hpp"
#include "roq/side.hpp"
#include "roq/time_in_force.hpp"

#include "roq/hyperliquid/json/order_status.hpp"
#include "roq/hyperliquid/json/side.hpp"
#include "roq/hyperliquid/json/time_in_force.hpp"

namespace roq {

// hyperliquid::json => roq

template <>
template <>
std::optional<Side> Map<hyperliquid::json::Side>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<hyperliquid::json::OrderStatus>::helper() const;

// roq => hyperliquid::json

template <>
template <>
std::optional<hyperliquid::json::Side> Map<Side>::helper() const;

template <>
template <>
std::optional<hyperliquid::json::TimeInForce> Map<TimeInForce>::helper() const;

}  // namespace roq
