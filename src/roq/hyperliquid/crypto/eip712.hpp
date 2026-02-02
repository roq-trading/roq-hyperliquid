#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <vector>

namespace roq {
namespace hyperliquid {
namespace crypto {

std::vector<uint8_t> encodeTypedData(nlohmann::json const &typed_data);

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
