#pragma once

#include <nlohmann/json.hpp>

#include <memory>
#include <vector>

#include "roq/hyperliquid/crypto/types.hpp"
#include "roq/hyperliquid/crypto/wallet.hpp"

namespace roq {
namespace hyperliquid {
namespace crypto {

struct Wallet;

/**
 * Wallet class for managing private keys and signing
 */
/*
class Wallet {
 public:
   // Create wallet from hex private key (with or without "0x" prefix)
  static std::shared_ptr<Wallet> fromPrivateKey(std::string const &private_key_hex);

   // Get the Ethereum address derived from this wallet's public key
  std::string address() const;

   // Sign a message hash with ECDSA
  Signature signMessage(std::vector<uint8_t> const &message_hash) const;

  ~Wallet();

 private:
  explicit Wallet(void *ec_key);  // EC_KEY* hidden from header

  void *ec_key_;  // OpenSSL EC_KEY*
  std::string address_;
};
*/

/**
 * Sign an L1 action (orders, cancels, etc.) using EIP-712
 * Note: Uses ordered_json to preserve key insertion order for msgpack serialization
 */
Signature signL1Action(
    Wallet const &wallet,
    nlohmann::ordered_json const &action,
    std::optional<std::string> const &vault_address,
    int64_t nonce,
    std::optional<int64_t> expires_after,
    bool is_mainnet);

Signature ROQ_signL1Action(
    Wallet const &wallet,
    std::vector<uint8_t> const &action_hash,
    std::optional<std::string> const &vault_address,
    int64_t nonce,
    std::optional<int64_t> expires_after,
    bool is_mainnet);

/**
 * Sign a user-signed action (transfers, etc.) using EIP-712
 */
Signature signUserSignedAction(
    Wallet const &wallet, nlohmann::json action, std::vector<EIP712Type> const &payload_types, std::string const &primary_type, bool is_mainnet);

/**
 * Compute action hash: keccak256(msgpack(action) + nonce + vault + expires)
 * Note: Uses ordered_json to preserve key insertion order for msgpack serialization
 */
std::vector<uint8_t> actionHash(
    nlohmann::ordered_json const &action, std::optional<std::string> const &vault_address, int64_t nonce, std::optional<int64_t> expires_after);

/**
 * Construct phantom agent for L1 action signing
 */
nlohmann::json constructPhantomAgent(std::vector<uint8_t> const &hash, bool is_mainnet);

/**
 * Create EIP-712 payload for L1 actions
 */
nlohmann::json l1Payload(nlohmann::json const &phantom_agent);

/**
 * Create EIP-712 payload for user-signed actions
 */
nlohmann::json userSignedPayload(std::string const &primary_type, std::vector<EIP712Type> const &payload_types, nlohmann::json const &action);

/**
 * Convert OrderRequest to OrderWire format
 */
OrderWire orderRequestToOrderWire(OrderRequest const &order, int asset);

/**
 * Create order action from order wires
 * Returns ordered_json to preserve key insertion order for L1 action signing
 */
nlohmann::ordered_json orderWiresToOrderAction(
    std::vector<OrderWire> const &order_wires, std::optional<BuilderInfo> const &builder, std::string const &grouping);

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
