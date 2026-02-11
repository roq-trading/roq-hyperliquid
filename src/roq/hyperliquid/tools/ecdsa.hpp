#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace roq {
namespace hyperliquid {
namespace tools {

struct ECDSA final {
  static std::string signHash(void const *ec_key, std::span<std::byte const> const &hash);
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
