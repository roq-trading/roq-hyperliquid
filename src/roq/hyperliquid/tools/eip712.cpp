#include "roq/hyperliquid/tools/eip712.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
auto keccak256_helper(auto &hasher, auto &payload) {
  hasher.clear();
  hasher.update(payload);
  std::vector<std::byte> result(32);
  hasher.final(result);
  return result;
}

std::vector<std::byte> hexToBytes(std::string const &hex) {
  std::string hex_str = hex;

  // Remove "0x" prefix if present
  if (hex_str.length() >= 2 && hex_str.substr(0, 2) == "0x") {
    hex_str = hex_str.substr(2);
  }

  // Hex string must have even length
  if (hex_str.length() % 2 != 0) {
    throw RuntimeError{"Hex string must have even length"sv};
  }

  std::vector<std::byte> bytes;
  bytes.reserve(hex_str.length() / 2);

  for (size_t i = 0; i < hex_str.length(); i += 2) {
    std::string byte_str = hex_str.substr(i, 2);
    auto byte = static_cast<std::byte>(std::stoul(byte_str, nullptr, 16));
    bytes.push_back(byte);
  }

  return bytes;
}

struct EIP712Type {
  std::string name;
  std::string type;

  nlohmann::json toJson() const { return {{"name", name}, {"type", type}}; }
};

std::string encodeType(std::string const &primary_type, std::map<std::string, std::vector<EIP712Type>> const &types) {
  auto it = types.find(primary_type);
  if (it == types.end()) {
    throw RuntimeError{"Primary type not found in types map"sv};
  }

  std::string result = primary_type + "(";
  auto const &fields = it->second;
  for (size_t i = 0; i < fields.size(); ++i) {
    if (i > 0) {
      result += ",";
    }
    result += fields[i].type + " " + fields[i].name;
  }
  result += ")";
  return result;
}

std::vector<std::byte> hashType(auto &hasher, std::string const &primary_type, std::map<std::string, std::vector<EIP712Type>> const &types) {
  std::string encoded = encodeType(primary_type, types);
  return keccak256_helper(hasher, encoded);
}

std::vector<std::byte> encodeField(auto &hasher, std::string const &type, nlohmann::json const &value) {
  // std::vector<std::byte> encoded(32, 0);
  std::vector<std::byte> encoded(32);

  if (type == "string") {
    // Hash the string value
    std::string str = value.get<std::string>();
    auto hash = keccak256_helper(hasher, str);
    return hash;
  } else if (type == "bytes32") {
    // Convert hex string to bytes
    std::string hex_str = value.get<std::string>();
    auto bytes = hexToBytes(hex_str);
    if (bytes.size() != 32) {
      throw RuntimeError{"bytes32 field must be 32 bytes"sv};
    }
    return bytes;
  } else if (type == "uint64") {
    // Encode as big-endian 32 bytes
    uint64_t num = value.get<uint64_t>();
    for (int i = 7; i >= 0; --i) {
      encoded[24 + (7 - i)] = static_cast<std::byte>((num >> (i * 8)) & 0xFF);
    }
    return encoded;
  } else if (type == "uint256") {
    // Encode as big-endian 32 bytes
    // Can handle both uint64 and larger numbers stored as uint64
    uint64_t num = value.get<uint64_t>();
    for (int i = 7; i >= 0; --i) {
      encoded[24 + (7 - i)] = static_cast<std::byte>((num >> (i * 8)) & 0xFF);
    }
    return encoded;
  } else if (type == "address") {
    // Address is 20 bytes, left-padded to 32 bytes
    std::string addr = value.get<std::string>();
    auto bytes = hexToBytes(addr);
    if (bytes.size() != 20) {
      throw RuntimeError{"Address must be 20 bytes"sv};
    }
    std::copy(bytes.begin(), bytes.end(), encoded.begin() + 12);
    return encoded;
  } else {
    throw RuntimeError{"Unsupported EIP-712 type: {}"sv, type};
  }
}

std::vector<std::byte> hashStruct(
    auto &hasher, std::string const &struct_type, nlohmann::json const &data, std::map<std::string, std::vector<EIP712Type>> const &types) {
  // Start with type hash
  std::vector<std::byte> encoded = hashType(hasher, struct_type, types);

  // Append encoded field values
  auto it = types.find(struct_type);
  if (it == types.end()) {
    throw RuntimeError{"Struct type not found in types map"sv};
  }

  auto const &fields = it->second;
  for (auto const &field : fields) {
    if (!data.contains(field.name)) {
      throw RuntimeError{"Missing field in struct data: {}"sv, field.name};
    }
    std::vector<std::byte> field_bytes = encodeField(hasher, field.type, data[field.name]);
    encoded.insert(encoded.end(), field_bytes.begin(), field_bytes.end());
  }

  return keccak256_helper(hasher, encoded);
}

}  // namespace

// === IMPLEMENTATION ===

std::vector<std::byte> EIP712::encodeTypedData(utils::hash::Keccak256 &hasher, nlohmann::json const &typed_data) {
  // Extract components
  if (!typed_data.contains("types") || !typed_data.contains("domain") || !typed_data.contains("primaryType") || !typed_data.contains("message")) {
    throw RuntimeError{"Invalid EIP-712 typed data structure"sv};
  }

  // Build types map
  std::map<std::string, std::vector<EIP712Type>> types_map;
  for (auto &[type_name, fields] : typed_data["types"].items()) {
    std::vector<EIP712Type> field_list;
    for (auto &field : fields) {
      field_list.push_back({field["name"], field["type"]});
    }
    types_map[type_name] = field_list;
  }

  // EIP-712 prefix
  std::vector<std::byte> result = {std::byte{0x19}, std::byte{0x01}};

  // Domain separator
  auto domain_hash = hashStruct(hasher, "EIP712Domain", typed_data["domain"], types_map);
  result.insert(result.end(), domain_hash.begin(), domain_hash.end());

  // Message hash
  std::string primary_type = typed_data["primaryType"];
  auto message_hash = hashStruct(hasher, primary_type, typed_data["message"], types_map);
  result.insert(result.end(), message_hash.begin(), message_hash.end());

  return keccak256_helper(hasher, result);
}
}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
