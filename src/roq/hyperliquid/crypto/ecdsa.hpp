#pragma once

#include <string>

#include "roq/hyperliquid/crypto/types.hpp"

namespace roq {
namespace hyperliquid {
namespace crypto {

void *createKeyFromPrivate(std::string const &private_key_hex);
std::string deriveAddress(void const *ec_key);
Signature signHash(void const *ec_key, std::vector<uint8_t> const &hash);

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
