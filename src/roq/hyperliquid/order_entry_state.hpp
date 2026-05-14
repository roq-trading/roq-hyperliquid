/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace hyperliquid {

enum class OrderEntryState : uint8_t {
  UNDEFINED = 0,
  SPOT_CLEARING_HOUSE_STATE,
  CLEARING_HOUSE_STATE,
  OPEN_ORDERS,
  USER_FILLS,
  DONE,
};

}  // namespace hyperliquid
}  // namespace roq
