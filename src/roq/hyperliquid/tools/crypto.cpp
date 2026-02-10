/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/crypto.hpp"

#include "roq/utils/codec/hex.hpp"

#include "roq/hyperliquid/crypto/constants.hpp"  // XXX FIXME TOD mainnet/testnet
#include "roq/hyperliquid/crypto/signing.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
auto compute_connection_id(auto &packed, auto &hash, auto &digest) {
  std::span<std::byte const> packed_2{reinterpret_cast<std::byte const *>(std::data(packed)), std::size(packed)};  // XXX FIXME TODO interface
  hash.clear();
  hash.update(packed_2);
  auto digest_2 = hash.final(digest);
  std::string result;
  utils::codec::Hex::encode(result, digest_2);
  result.insert(0, "0x"sv);  // XXX FIXME TODO inefficient
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &key, std::string_view const &secret) : key_{key}, wallet_{secret} {
  log::warn("DEBUG wallet address={}"sv, wallet_.address());
}

std::string Crypto::sign(std::string_view const &action, std::vector<uint8_t> const &packed, std::chrono::milliseconds now_utc) {
  auto connection_id = compute_connection_id(packed, hash_, digest_);
  //
  std::optional<std::string> vault_opt = std::nullopt;
  std::optional<int64_t> expires_after;
  auto is_mainnet = true;  // XXX FIXME TODO testnet
  std::string source = is_mainnet ? "a" : "b";
  auto domain = nlohmann::json::array(
      {{{"name", "name"}, {"type", "string"}},
       {{"name", "version"}, {"type", "string"}},
       {{"name", "chainId"}, {"type", "uint256"}},
       {{"name", "verifyingContract"}, {"type", "address"}}});
  auto agent = nlohmann::json::array({{{"name", "source"}, {"type", "string"}}, {{"name", "connectionId"}, {"type", "bytes32"}}});
  auto phantom_agent = nlohmann::json{
      {"source", source},
      {"connectionId", connection_id},
  };
  auto payload = nlohmann::json{
      {"domain", {{"name", "Exchange"}, {"version", "1"}, {"chainId", 1337}, {"verifyingContract", "0x0000000000000000000000000000000000000000"}}},
      {"primaryType", "Agent"},
      {"types", {{"EIP712Domain", domain}, {"Agent", agent}}},
      {"message", phantom_agent},
  };
  auto signature = ROQ_signL1Action(wallet_, payload, vault_opt, now_utc.count(), expires_after, is_mainnet);  // XXX FIXME TODO move to tools
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
