#include "roq/hyperliquid/crypto/signing.hpp"

#include <sstream>
#include <stdexcept>

#include "roq/hyperliquid/crypto/conversions.hpp"
#include "roq/hyperliquid/crypto/ecdsa.hpp"
#include "roq/hyperliquid/crypto/eip712.hpp"
#include "roq/hyperliquid/crypto/keccak.hpp"
#include "roq/hyperliquid/crypto/wallet.hpp"

#define MSGPACK_NO_BOOST
#include <msgpack.hpp>

namespace roq {
namespace hyperliquid {
namespace crypto {

// Forward declarations from crypto namespace
namespace crypto {
// void *createKeyFromPrivate(std::string const &private_key_hex);
// std::string deriveAddress(void const *ec_key);
// Signature signHash(void const *ec_key, std::vector<uint8_t> const &hash);
void freeKey(void *ec_key);
// std::vector<uint8_t> encodeTypedData(nlohmann::json const &typed_data);
}  // namespace crypto

// Helper function to pack JSON to msgpack (works with both json and ordered_json)
template <typename JsonType>
static void packJsonImpl(msgpack::packer<std::stringstream> &packer, JsonType const &j) {
  if (j.is_null()) {
    packer.pack_nil();
  } else if (j.is_boolean()) {
    packer.pack(j.template get<bool>());
  } else if (j.is_number_integer()) {
    packer.pack(j.template get<int64_t>());
  } else if (j.is_number_unsigned()) {
    packer.pack(j.template get<uint64_t>());
  } else if (j.is_number_float()) {
    packer.pack(j.template get<double>());
  } else if (j.is_string()) {
    packer.pack(j.template get<std::string>());
  } else if (j.is_array()) {
    packer.pack_array(j.size());
    for (auto const &item : j) {
      packJsonImpl(packer, item);
    }
  } else if (j.is_object()) {
    packer.pack_map(j.size());
    // ordered_json preserves insertion order when iterating
    for (auto it = j.begin(); it != j.end(); ++it) {
      packer.pack(it.key());
      packJsonImpl(packer, it.value());
    }
  }
}

// Wrapper functions for different JSON types
static void packJson(msgpack::packer<std::stringstream> &packer, nlohmann::json const &j) {
  packJsonImpl(packer, j);
}

static void packJson(msgpack::packer<std::stringstream> &packer, nlohmann::ordered_json const &j) {
  packJsonImpl(packer, j);
}

// Wallet implementation

/*
Wallet::Wallet(void *ec_key) : ec_key_(ec_key) {
  address_ = crypto::deriveAddress(ec_key_);
}

Wallet::~Wallet() {
  crypto::freeKey(ec_key_);
}

std::shared_ptr<Wallet> Wallet::fromPrivateKey(std::string const &private_key_hex) {
  void *ec_key = crypto::createKeyFromPrivate(private_key_hex);
  return std::shared_ptr<Wallet>(new Wallet(ec_key));
}

std::string Wallet::address() const {
  return address_;
}

Signature Wallet::signMessage(std::vector<uint8_t> const &message_hash) const {
  return crypto::signHash(ec_key_, message_hash);
}
*/

// Action hash computation

std::vector<uint8_t> actionHash(
    nlohmann::ordered_json const &action, std::optional<std::string> const &vault_address, int64_t nonce, std::optional<int64_t> expires_after) {
  std::vector<uint8_t> data;

  // 1. Msgpack serialize the action
  std::stringstream ss;
  msgpack::packer<std::stringstream> packer(ss);
  packJson(packer, action);
  std::string msgpack_str = ss.str();
  data.insert(data.end(), msgpack_str.begin(), msgpack_str.end());

  // 2. Append nonce (8 bytes, big-endian)
  for (int i = 7; i >= 0; --i) {
    data.push_back(static_cast<uint8_t>((nonce >> (i * 8)) & 0xFF));
  }

  // 3. Append vault address if present
  if (!vault_address.has_value()) {
    data.push_back(0x00);
  } else {
    data.push_back(0x01);
    std::vector<uint8_t> addr_bytes = hexToBytes(vault_address.value());
    if (addr_bytes.size() != 20) {
      throw std::runtime_error("Invalid vault address length");
    }
    data.insert(data.end(), addr_bytes.begin(), addr_bytes.end());
  }

  // 4. Append expires_after if present
  if (expires_after.has_value()) {
    data.push_back(0x00);
    int64_t expires = expires_after.value();
    for (int i = 7; i >= 0; --i) {
      data.push_back(static_cast<uint8_t>((expires >> (i * 8)) & 0xFF));
    }
  }

  // 5. Hash with Keccak-256
  return keccak256(data);
}

// Phantom agent construction

nlohmann::json constructPhantomAgent(std::vector<uint8_t> const &hash, bool is_mainnet) {
  std::string source = is_mainnet ? "a" : "b";
  std::string connection_id = bytesToHex(hash, true);

  return {{"source", source}, {"connectionId", connection_id}};
}

nlohmann::json ROQ_constructPhantomAgent(std::string_view const &hash, bool is_mainnet) {
  std::string source = is_mainnet ? "a" : "b";
  // std::string connection_id = ROQ_bytesToHex(hash, true);

  return {{"source", source}, {"connectionId", hash}};
}

// L1 payload for EIP-712

nlohmann::json l1Payload(nlohmann::json const &phantom_agent) {
  nlohmann::json payload = {
      {"domain", {{"name", "Exchange"}, {"version", "1"}, {"chainId", 1337}, {"verifyingContract", "0x0000000000000000000000000000000000000000"}}},
      {"primaryType", "Agent"},
      {"types",
       {{"EIP712Domain",
         nlohmann::json::array(
             {{{"name", "name"}, {"type", "string"}},
              {{"name", "version"}, {"type", "string"}},
              {{"name", "chainId"}, {"type", "uint256"}},
              {{"name", "verifyingContract"}, {"type", "address"}}})},
        {"Agent", nlohmann::json::array({{{"name", "source"}, {"type", "string"}}, {{"name", "connectionId"}, {"type", "bytes32"}}})}}},
      {"message", phantom_agent}};

  return payload;
}

// User-signed payload for EIP-712

nlohmann::json userSignedPayload(std::string const &primary_type, std::vector<EIP712Type> const &payload_types, nlohmann::json const &action) {
  nlohmann::json types_array = nlohmann::json::array();
  for (auto const &type : payload_types) {
    types_array.push_back(type.toJson());
  }

  // Extract chain ID from action's signatureChainId field
  if (!action.contains("signatureChainId")) {
    throw std::runtime_error("Action must contain signatureChainId field");
  }
  std::string chain_id_hex = action["signatureChainId"].get<std::string>();
  // Remove "0x" prefix if present
  if (chain_id_hex.substr(0, 2) == "0x") {
    chain_id_hex = chain_id_hex.substr(2);
  }
  int64_t chain_id = std::stoll(chain_id_hex, nullptr, 16);

  nlohmann::json payload = {
      {"domain",
       {{"name", "HyperliquidSignTransaction"}, {"version", "1"}, {"chainId", chain_id}, {"verifyingContract", "0x0000000000000000000000000000000000000000"}}},
      {"primaryType", primary_type},
      {"types",
       {{"EIP712Domain",
         nlohmann::json::array(
             {{{"name", "name"}, {"type", "string"}},
              {{"name", "version"}, {"type", "string"}},
              {{"name", "chainId"}, {"type", "uint256"}},
              {{"name", "verifyingContract"}, {"type", "address"}}})},
        {primary_type, types_array}}},
      {"message", action}};

  return payload;
}

// Sign L1 action

Signature signL1Action(
    Wallet const &wallet,
    nlohmann::ordered_json const &action,
    std::optional<std::string> const &vault_address,
    int64_t nonce,
    std::optional<int64_t> expires_after,
    bool is_mainnet) {
  // Compute action hash
  auto hash = actionHash(action, vault_address, nonce, expires_after);

  // Construct phantom agent
  auto phantom_agent = constructPhantomAgent(hash, is_mainnet);

  // Create EIP-712 payload
  auto payload = l1Payload(phantom_agent);

  // Encode typed data
  auto message_hash = encodeTypedData(payload);

  // Sign the hash
  return wallet.signMessage(message_hash);
}

// HANS
std::string ROQ_signL1Action(Wallet const &wallet, nlohmann::json const &payload) {
  // Construct phantom agent
  // auto phantom_agent = ROQ_constructPhantomAgent(action_hash, is_mainnet);

  // Create EIP-712 payload
  // auto payload = l1Payload(phantom_agent);

  // Encode typed data
  auto message_hash = encodeTypedData(payload);

  // Sign the hash
  auto signature = wallet.signMessage(message_hash);

  return signature.toJson().dump();
}

// Sign user-signed action

Signature signUserSignedAction(
    Wallet const &wallet, nlohmann::json action, std::vector<EIP712Type> const &payload_types, std::string const &primary_type, bool is_mainnet) {
  // Add signatureChainId and hyperliquidChain to action
  action["signatureChainId"] = "0x66eee";
  action["hyperliquidChain"] = is_mainnet ? "Mainnet" : "Testnet";

  // Create EIP-712 payload
  auto payload = userSignedPayload(primary_type, payload_types, action);

  // Encode typed data
  auto message_hash = encodeTypedData(payload);

  // Sign the hash
  return wallet.signMessage(message_hash);
}

// Order conversion

OrderWire orderRequestToOrderWire(OrderRequest const &order, int asset) {
  OrderWire wire;
  wire.asset = asset;
  wire.is_buy = order.is_buy;
  wire.price = floatToWire(order.limit_px);
  wire.size = floatToWire(order.sz);
  // wire.price = std::empty(order.ROQ_price) ? floatToWire(order.limit_px) : order.ROQ_price;
  // wire.size = std::empty(order.ROQ_quantity) ? floatToWire(order.sz) : order.ROQ_quantity;
  wire.reduce_only = order.reduce_only;

  // Convert order type
  if (order.order_type.limit.has_value()) {
    wire.order_type = {
        {"limit",
         {
             {"tif", order.order_type.limit->tif},
         }},
    };
  } else if (order.order_type.trigger.has_value()) {
    wire.order_type = {
        {"trigger",
         {
             {"triggerPx", floatToWire(order.order_type.trigger->trigger_px)},
             {"isMarket", order.order_type.trigger->is_market},
             {"tpsl", order.order_type.trigger->tpsl},
         }},
    };
  }

  // Add cloid if present
  if (order.cloid.has_value()) {
    wire.cloid = order.cloid->toRaw();
  }

  return wire;
}

// Create order action

nlohmann::ordered_json orderWiresToOrderAction(
    std::vector<OrderWire> const &order_wires, std::optional<BuilderInfo> const &builder, std::string const &grouping) {
  nlohmann::ordered_json orders_array = nlohmann::ordered_json::array();
  for (auto const &wire : order_wires) {
    nlohmann::ordered_json wire_ordered(wire.toJson());
    orders_array.push_back(std::move(wire_ordered));
  }

  // Use ordered_json and insert keys in the correct order
  nlohmann::ordered_json action;
  action["type"] = "order";
  action["orders"] = orders_array;
  action["grouping"] = grouping;

  if (builder.has_value()) {
    nlohmann::ordered_json builder_obj;
    builder_obj["b"] = builder->b;
    builder_obj["f"] = builder->f;
    action["builder"] = builder_obj;
  }

  return action;
}

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
