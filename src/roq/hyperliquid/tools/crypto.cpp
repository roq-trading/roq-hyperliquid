/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/crypto.hpp"

#include "roq/logging.hpp"

#include "roq/utils/codec/hex.hpp"

#include "roq/hyperliquid/tools/eip712.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
auto compute_connection_id(auto &packed, auto &hash, auto &digest) {
  hash.clear();
  hash.update(packed);
  auto digest_2 = hash.final(digest);
  std::string result;
  utils::codec::Hex::encode(result, digest_2);
  result.insert(0, "0x"sv);  // XXX FIXME TODO inefficient
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &key, std::string_view const &secret) : key_{key}, mainnet_{true}, wallet_{secret} {
  log::warn("DEBUG wallet address={}"sv, wallet_.get_address());
}

std::string Crypto::sign_l1_action(
    std::string_view const &action, std::span<std::byte const> const &packed, std::chrono::milliseconds now_utc, std::chrono::milliseconds expires_after_utc) {
  auto source = mainnet_ ? "a" : "b";
  auto connection_id = compute_connection_id(packed, hash_, digest_);
  auto domain = nlohmann::json{
      {"name", "Exchange"},
      {"version", "1"},
      {"chainId", 1337},
      {"verifyingContract", "0x0000000000000000000000000000000000000000"},
  };
  auto EIP712_domain = nlohmann::json::array({
      {{"name", "name"}, {"type", "string"}},
      {{"name", "version"}, {"type", "string"}},
      {{"name", "chainId"}, {"type", "uint256"}},
      {{"name", "verifyingContract"}, {"type", "address"}},
  });
  auto agent = nlohmann::json::array({
      {{"name", "source"}, {"type", "string"}},
      {{"name", "connectionId"}, {"type", "bytes32"}},
  });
  auto types = nlohmann::json{
      {"EIP712Domain", EIP712_domain},
      {"Agent", agent},
  };
  auto phantom_agent = nlohmann::json{
      {"source", source},
      {"connectionId", connection_id},
  };
  auto payload = nlohmann::json{
      {"domain", domain},
      {"primaryType", "Agent"},
      {"types", types},
      {"message", phantom_agent},
  };
  auto hash = EIP712::encodeTypedData(hash_, payload);
  auto signature = wallet_.sign_ecdsa(hash);
  auto result = fmt::format(
      R"({{)"
      R"("action":{},)"
      R"("nonce":{},)"
      R"("signature":{},)"
      R"("expiresAfter":{})"
      R"(}})"sv,
      action,
      now_utc.count(),
      signature,
      expires_after_utc.count());
  return result;
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
