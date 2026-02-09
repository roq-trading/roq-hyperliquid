/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/crypto.hpp"

#include "roq/hyperliquid/crypto/constants.hpp"  // XXX FIXME TOD mainnet/testnet
#include "roq/hyperliquid/crypto/signing.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &key, std::string_view const &secret) : key_{key}, wallet_{secret} {
  log::warn("DEBUG wallet address={}"sv, wallet_.address());
}

std::string Crypto::sign(std::string_view const &action, std::vector<uint8_t> const &hash, std::chrono::milliseconds now_utc) {
  std::optional<std::string> vault_opt = std::nullopt;
  std::optional<int64_t> expires_after;
  auto is_mainnet = true;                                                                                   // XXX FIXME TODO testnet
  auto signature = ROQ_signL1Action(wallet_, hash, vault_opt, now_utc.count(), expires_after, is_mainnet);  // XXX FIXME TODO move to tools
  auto result = fmt::format(
      R"({{)"
      R"("action":{},)"
      R"("nonce":{},)"
      R"("signature":{})"
      R"(}})"sv,
      action,
      now_utc.count(),
      signature);
  return result;
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
