#include "roq/hyperliquid/tools/ecdsa.hpp"

// #include <openssl/bn.h>
#include <openssl/core_names.h>
// #include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/obj_mac.h>
#include <openssl/param_build.h>

#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "roq/exceptions.hpp"

#include "roq/hyperliquid/tools/bignum.hpp"
#include "roq/hyperliquid/tools/group.hpp"
#include "roq/hyperliquid/tools/point.hpp"
#include "roq/hyperliquid/tools/signature.hpp"

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
std::string bnToHex(BigNum const &bn, size_t min_bytes = 32) {
  auto num_bytes = std::size(bn);
  std::vector<std::byte> bytes(std::max(num_bytes, min_bytes), std::byte{0x00});

  // big-endian bytes
  [[maybe_unused]] auto tmp = bn.to_binary(bytes);  // XXX FIXME TODO use tmp ???

  std::ostringstream oss;
  oss << std::hex << std::setfill('0');

  // Skip leading zeros (but keep at least one byte)
  size_t start = 0;
  while (start < std::size(bytes) - 1 && bytes[start] == std::byte{0x00}) {
    start++;
  }

  for (size_t i = start; i < std::size(bytes); i++) {
    oss << std::setw(2) << static_cast<int>(bytes[i]);
  }
  return oss.str();
}

// RFC 6979 deterministic k generation
BigNum generateDeterministicK(BigNum const &priv_key, std::span<std::byte const> const &hash, Group const &group) {
  // Get curve order
  auto order = group.get_order();

  // Convert private key and hash to bytes
  std::vector<std::byte> hash_bytes;
  for (auto b : hash) {
    hash_bytes.emplace_back(b);
  }

  std::vector<std::byte> priv_bytes(32);
  priv_key.to_binary(priv_bytes);

  // Ensure hash is 32 bytes
  if (std::size(hash_bytes) > 32) {
    hash_bytes.resize(32);
  } else {
    while (std::size(hash_bytes) < 32) {
      hash_bytes.insert(hash_bytes.begin(), std::byte{0x00});
    }
  }

  // RFC 6979 Section 3.2
  // Step a: hash message (already done)
  // Step b: h1 = H(m) truncated to qlen bits

  // Step c: V = 0x01 0x01 ...0x01 (32 bytes)
  std::vector<std::byte> V(32, std::byte{0x01});

  // Step d: K = 0x00 0x00 ... 0x00 (32 bytes)
  std::vector<std::byte> K(32);

  // Step e: K = HMAC_K(V || 0x00 || priv || hash)
  std::vector<std::byte> data;
  data.insert(data.end(), V.begin(), V.end());
  data.push_back(std::byte{0x00});
  data.insert(data.end(), priv_bytes.begin(), priv_bytes.end());
  data.insert(data.end(), hash_bytes.begin(), hash_bytes.end());

  unsigned int len;
  HMAC(
      EVP_sha256(),
      reinterpret_cast<unsigned char const *>(std::data(K)),
      std::size(K),
      reinterpret_cast<unsigned char const *>(std::data(data)),
      std::size(data),
      reinterpret_cast<unsigned char *>(std::data(K)),
      &len);

  // Step f: V = HMAC_K(V)
  HMAC(
      EVP_sha256(),
      reinterpret_cast<unsigned char const *>(std::data(K)),
      std::size(K),
      reinterpret_cast<unsigned char const *>(std::data(V)),
      std::size(V),
      reinterpret_cast<unsigned char *>(std::data(V)),
      &len);

  // Step g: K = HMAC_K(V || 0x01 || priv || hash)
  data.clear();
  data.insert(data.end(), V.begin(), V.end());
  data.push_back(std::byte{0x01});
  data.insert(data.end(), priv_bytes.begin(), priv_bytes.end());
  data.insert(data.end(), hash_bytes.begin(), hash_bytes.end());

  HMAC(
      EVP_sha256(),
      reinterpret_cast<unsigned char const *>(std::data(K)),
      std::size(K),
      reinterpret_cast<unsigned char const *>(std::data(data)),
      std::size(data),
      reinterpret_cast<unsigned char *>(std::data(K)),
      &len);

  // Step h: V = HMAC_K(V)
  HMAC(
      EVP_sha256(),
      reinterpret_cast<unsigned char const *>(std::data(K)),
      std::size(K),
      reinterpret_cast<unsigned char const *>(std::data(V)),
      std::size(V),
      reinterpret_cast<unsigned char *>(std::data(V)),
      &len);

  // Step h3: Generate k
  while (true) {
    // T = V = HMAC_K(V)
    HMAC(
        EVP_sha256(),
        reinterpret_cast<unsigned char const *>(std::data(K)),
        std::size(K),
        reinterpret_cast<unsigned char const *>(std::data(V)),
        std::size(V),
        reinterpret_cast<unsigned char *>(std::data(V)),
        &len);

    auto k = BigNum::create_from_binary(V);

    // Check if k is in [1, order-1]
    if (std::empty(k) || k.compare(order) >= 0) {
      // K = HMAC_K(V || 0x00)
      data.clear();
      data.insert(data.end(), V.begin(), V.end());
      data.push_back(std::byte{0x00});
      HMAC(
          EVP_sha256(),
          reinterpret_cast<unsigned char const *>(std::data(K)),
          std::size(K),
          reinterpret_cast<unsigned char const *>(std::data(data)),
          std::size(data),
          reinterpret_cast<unsigned char *>(std::data(K)),
          &len);

      // V = HMAC_K(V)
      HMAC(
          EVP_sha256(),
          reinterpret_cast<unsigned char const *>(std::data(K)),
          std::size(K),
          reinterpret_cast<unsigned char const *>(std::data(V)),
          std::size(V),
          reinterpret_cast<unsigned char *>(std::data(V)),
          &len);
    } else {
      return k;
    }
  }
}

int calculateRecoveryId(Key const &key, std::span<std::byte const> const &hash, const ECDSA_SIG *sig) {
  auto group = key.get_group();
  auto pub_key = key.get_public_key();

  const BIGNUM *r, *s;
  ECDSA_SIG_get0(sig, &r, &s);

  // Get the order of the curve
  auto order = group.get_order();

  // Get curve parameters
  auto p = group.get_curve();

  BN_CTX *ctx = BN_CTX_new();

  // Try both recovery IDs (0 and 1)
  for (int recovery_id = 0; recovery_id < 2; ++recovery_id) {
    // Calculate x coordinate of R (which is r)
    BigNum x{r};

    // Calculate y from x
    // y^2 = x^3 + 7 (for secp256k1)
    BigNum y_squared;
    BigNum y;
    BigNum tmp;

    // y_squared = x^3
    BN_mod_mul(tmp, x, x, p, ctx);
    BN_mod_mul(y_squared, tmp, x, p, ctx);

    // y_squared = x^3 + 7
    BN_add_word(y_squared, 7);
    BN_mod(y_squared, y_squared, p, ctx);

    // y = sqrt(y_squared) mod p
    BN_mod_sqrt(y, y_squared, p, ctx);

    // Choose y based on recovery_id (odd/even)
    if ((BN_is_odd(y) ? 1 : 0) != recovery_id) {
      BN_sub(y, p, y);
    }

    // Create point R = (x, y)
    auto R = Point::create_from_group(group);
    EC_POINT_set_affine_coordinates(group, R, x, y, ctx);

    // Recover public key: Q = r^-1 * (s*R - e*G)
    BigNum r_inv;
    BN_mod_inverse(r_inv, r, order, ctx);

    auto e = BigNum::create_from_binary(hash);

    auto sR = Point::create_from_group(group);
    EC_POINT_mul(group, sR, nullptr, R, s, ctx);

    auto eG = Point::create_from_group(group);
    EC_POINT_mul(group, eG, e, nullptr, nullptr, ctx);

    auto Q = Point::create_from_group(group);
    EC_POINT_invert(group, eG, ctx);
    EC_POINT_add(group, Q, sR, eG, ctx);
    EC_POINT_mul(group, Q, nullptr, Q, r_inv, ctx);

    // Compare recovered public key with actual public key
    int match = (EC_POINT_cmp(group, Q, pub_key, ctx) == 0);

    if (match) {
      BN_CTX_free(ctx);
      return recovery_id;
    }
  }

  BN_CTX_free(ctx);

  // Default to 0 if recovery fails
  return 0;
}
}  // namespace

// === IMPLEMENTATION ===

std::string ECDSA::signHash(Key const &key, std::span<std::byte const> const &hash) {
  if (std::size(hash) != 32) {
    throw RuntimeError{"Hash must be 32 bytes"};
  }

  auto group = key.get_group();
  auto priv_key = key.get_private_key();

  // Generate deterministic k using RFC 6979
  auto k = generateDeterministicK(priv_key, hash, group);

  // Get curve order
  auto order = group.get_order();

  BN_CTX *ctx = BN_CTX_new();

  // Calculate r = (k * G).x mod order
  auto kG = Point::create_from_group(group);
  EC_POINT_mul(group, kG, k, nullptr, nullptr, ctx);

  BigNum r;
  BigNum y_coord;
  EC_POINT_get_affine_coordinates(group, kG, r, y_coord, ctx);
  BN_mod(r, r, order, ctx);

  // Calculate s = k^-1 * (hash + r * priv_key) mod order
  BigNum k_inv;
  BN_mod_inverse(k_inv, k, order, ctx);

  auto e = BigNum::create_from_binary(hash);
  BigNum s;
  BigNum tmp;

  BN_mod_mul(tmp, r, priv_key, order, ctx);  // r * priv_key
  BN_mod_add(tmp, e, tmp, order, ctx);       // hash + r * priv_key
  BN_mod_mul(s, k_inv, tmp, order, ctx);     // k^-1 * (hash + r * priv_key)

  // Ensure s is in the lower half (ETH requirement for non-malleability)
  BigNum half_order;
  BN_rshift1(half_order, order);
  if (s.compare(half_order) > 0) {
    BN_sub(s, order, s);
  }

  // Create signature
  ECDSA_SIG *sig = ECDSA_SIG_new();
  ECDSA_SIG_set0(sig, BN_dup(r), BN_dup(s));

  // Convert to hex strings
  Signature result;
  result.r = "0x" + bnToHex(r, 32);
  result.s = "0x" + bnToHex(s, 32);

  // Calculate recovery ID (v)
  int recovery_id = calculateRecoveryId(key, hash, sig);
  result.v = recovery_id + 27;  // Ethereum uses 27/28

  // Cleanup
  ECDSA_SIG_free(sig);
  BN_CTX_free(ctx);

  return result.toJson().dump();
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
