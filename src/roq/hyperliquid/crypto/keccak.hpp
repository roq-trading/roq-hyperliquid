#pragma once

#include <cstdint>
#include <vector>

namespace roq {
namespace hyperliquid {
namespace crypto {

std::vector<uint8_t> keccak256(uint8_t const *data, size_t len);

inline std::vector<uint8_t> keccak256(std::vector<uint8_t> const &data) {
  return keccak256(std::data(data), std::size(data));
}

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
