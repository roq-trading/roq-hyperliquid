/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace hyperliquid {

enum class RestState : uint8_t {
  UNDEFINED = 0,
  SPOT_META,
  PERP_DEXS,
  META,
  DONE,
};

}  // namespace hyperliquid
}  // namespace roq
