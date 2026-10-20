/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/crypto.hpp"

#include <fmt/ranges.h>

#include "roq/logging.hpp"

#include "roq/utils/codec/hex.hpp"

#include "roq/utils/eth/messages.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === CONSTANTS ===

namespace {
auto const EIP712_DOMAIN = utils::eth::Domain{
    .name = "Exchange"sv,
    .version = "1"sv,
    .chain_id = 1337,
    .verifying_contract = "0x0000000000000000000000000000000000000000"sv,
};

auto const SOURCE_MAINNET = "a"sv;
auto const SOURCE_TESTNET = "b"sv;
}  // namespace

// === HELPERS ===

namespace {
auto get_source(auto mainnet) {
  return mainnet ? SOURCE_MAINNET : SOURCE_TESTNET;
}

auto compute_connection_id(auto &packed, auto &hasher, auto &digest) {
  hasher.clear();
  hasher.update(packed);
  auto digest_2 = hasher.final(digest);
  return fmt::format("0x{}"sv, utils::codec::HexPrinter{digest_2});
}
}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &key, std::string_view const &secret) : key_{key}, mainnet_{true}, account_{secret} {
  log::warn("DEBUG wallet address={}"sv, account_.get_address());
}

std::string Crypto::sign_l1_action(
    std::string_view const &action, std::span<std::byte const> const &packed, std::chrono::milliseconds now_utc, std::chrono::milliseconds expires_after_utc) {
  // XXX FIXME TODO maybe we need to ensure uniqueness for the signer?
  if (nonce_ < now_utc) {
    nonce_ = now_utc;
  } else {
    ++nonce_;
  }
  auto connection_id = compute_connection_id(packed, hash_, digest_);
  std::array<utils::eth::Field, 2> data{{
      {
          .name = "source"sv,
          .type = utils::eth::Type::STRING,
          .value = get_source(mainnet_),
      },
      {
          .name = "connectionId"sv,
          .type = utils::eth::Type::BYTES32,
          .value = connection_id,
      },
  }};
  auto message = utils::eth::Message{
      .type = "Agent"sv,
      .data = data,
  };
  auto signable_message = utils::eth::Messages::encode_typed_data(hash_, EIP712_DOMAIN, message);
  auto signed_message = account_.sign_message(hash_, signable_message);
  return fmt::format(
      R"({{)"
      R"("action":{},)"
      R"("nonce":{},)"
      R"("signature":{{)"
      R"("v":{},)"
      R"("r":"0x{}",)"
      R"("s":"0x{}")"
      R"(}},)"
      R"("expiresAfter":{})"
      R"(}})"sv,
      action,
      nonce_.count(),
      signed_message.v(),
      utils::codec::HexPrinter{signed_message.r()},
      utils::codec::HexPrinter{signed_message.s()},
      expires_after_utc.count());
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
