#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <vector>

#include "roq/utils/hash/keccak256.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

struct EIP712 final {
  static std::vector<std::byte> encodeTypedData(utils::hash::Keccak256 &, nlohmann::json const &typed_data);
};

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
