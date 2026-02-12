#pragma once

#include <cstddef>
#include <span>
#include <string>

#include "roq/hyperliquid/tools/key.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct ECDSA final {
  static std::string signHash(Key const &, std::span<std::byte const> const &hash);
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
