/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/map.hpp"

#include "roq/side.hpp"

#include "roq/hyperliquid/json/side.hpp"

namespace roq {

template <>
template <>
std::optional<Side> Map<hyperliquid::json::Side>::helper() const;

}  // namespace roq
