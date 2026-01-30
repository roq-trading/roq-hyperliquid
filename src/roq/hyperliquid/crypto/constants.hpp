#pragma once

#include <string>

namespace roq {
namespace hyperliquid {
namespace crypto {

// API URLs
std::string const MAINNET_API_URL = "https://api.hyperliquid.xyz";
std::string const TESTNET_API_URL = "https://api.hyperliquid-testnet.xyz";
std::string const LOCAL_API_URL = "http://localhost:3001";

// Default timeout in milliseconds
int const DEFAULT_TIMEOUT_MS = 30000;

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
