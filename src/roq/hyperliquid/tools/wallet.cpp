/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/wallet.hpp"

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <iomanip>
#include <sstream>
#include <vector>

#include "roq/exceptions.hpp"

#include "roq/hyperliquid/tools/bignum.hpp"
#include "roq/hyperliquid/tools/point.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
std::string bnToHex(const BIGNUM *bn, int min_bytes = 32) {
  int num_bytes = BN_num_bytes(bn);
  std::vector<uint8_t> bytes(std::max(num_bytes, min_bytes), 0);

  // BN_bn2bin writes big-endian bytes
  int actual_bytes = BN_bn2bin(bn, bytes.data() + (bytes.size() - num_bytes));
  if (actual_bytes != num_bytes) {
    throw std::runtime_error("BN_bn2bin failed");
  }

  std::ostringstream oss;
  oss << std::hex << std::setfill('0');

  // Skip leading zeros (but keep at least one byte)
  size_t start = 0;
  while (start < bytes.size() - 1 && bytes[start] == 0) {
    start++;
  }

  for (size_t i = start; i < bytes.size(); i++) {
    oss << std::setw(2) << static_cast<int>(bytes[i]);
  }
  return oss.str();
}

// RFC 6979 deterministic k generation
BIGNUM *generateDeterministicK(const BIGNUM *priv_key, std::span<uint8_t const> const &hash, const EC_GROUP *group) {
  // Get curve order
  BigNum order;
  EC_GROUP_get_order(group, order, nullptr);

  // Convert private key and hash to bytes
  std::vector<uint8_t> priv_bytes(32);
  std::vector<uint8_t> hash_bytes;
  hash_bytes.assign_range(hash);

  BN_bn2binpad(priv_key, priv_bytes.data(), 32);

  // Ensure hash is 32 bytes
  if (hash_bytes.size() > 32) {
    hash_bytes.resize(32);
  } else {
    while (hash_bytes.size() < 32) {
      hash_bytes.insert(hash_bytes.begin(), 0);
    }
  }

  // RFC 6979 Section 3.2
  // Step a: hash message (already done)
  // Step b: h1 = H(m) truncated to qlen bits

  // Step c: V = 0x01 0x01 ...0x01 (32 bytes)
  std::vector<uint8_t> V(32, 0x01);

  // Step d: K = 0x00 0x00 ... 0x00 (32 bytes)
  std::vector<uint8_t> K(32, 0x00);

  // Step e: K = HMAC_K(V || 0x00 || priv || hash)
  std::vector<uint8_t> data;
  data.insert(data.end(), V.begin(), V.end());
  data.push_back(0x00);
  data.insert(data.end(), priv_bytes.begin(), priv_bytes.end());
  data.insert(data.end(), hash_bytes.begin(), hash_bytes.end());

  unsigned int len;
  HMAC(EVP_sha256(), K.data(), K.size(), data.data(), data.size(), K.data(), &len);

  // Step f: V = HMAC_K(V)
  HMAC(EVP_sha256(), K.data(), K.size(), V.data(), V.size(), V.data(), &len);

  // Step g: K = HMAC_K(V || 0x01 || priv || hash)
  data.clear();
  data.insert(data.end(), V.begin(), V.end());
  data.push_back(0x01);
  data.insert(data.end(), priv_bytes.begin(), priv_bytes.end());
  data.insert(data.end(), hash_bytes.begin(), hash_bytes.end());

  HMAC(EVP_sha256(), K.data(), K.size(), data.data(), data.size(), K.data(), &len);

  // Step h: V = HMAC_K(V)
  HMAC(EVP_sha256(), K.data(), K.size(), V.data(), V.size(), V.data(), &len);

  // Step h3: Generate k
  BIGNUM *k = nullptr;
  while (true) {
    // T = V = HMAC_K(V)
    HMAC(EVP_sha256(), K.data(), K.size(), V.data(), V.size(), V.data(), &len);

    k = BN_bin2bn(V.data(), V.size(), k);

    // Check if k is in [1, order-1]
    if (BN_is_zero(k) || BN_cmp(k, order) >= 0) {
      // K = HMAC_K(V || 0x00)
      data.clear();
      data.insert(data.end(), V.begin(), V.end());
      data.push_back(0x00);
      HMAC(EVP_sha256(), K.data(), K.size(), data.data(), data.size(), K.data(), &len);

      // V = HMAC_K(V)
      HMAC(EVP_sha256(), K.data(), K.size(), V.data(), V.size(), V.data(), &len);
    } else {
      break;
    }
  }

  // BN_free(order);
  return k;
}

int calculateRecoveryId(auto &key, std::span<uint8_t const> const &hash, const ECDSA_SIG *sig) {
  auto group = static_cast<EC_GROUP const *>(key);
  auto pub_key = key.get_public_key();

  const BIGNUM *r, *s;
  ECDSA_SIG_get0(sig, &r, &s);

  // Get the order of the curve
  BigNum order;
  EC_GROUP_get_order(group, order, nullptr);

  // Get curve parameters
  BigNum p;
  EC_GROUP_get_curve(group, p, nullptr, nullptr, nullptr);

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
    Point R{group};
    EC_POINT_set_affine_coordinates(group, R, x, y, ctx);

    // Recover public key: Q = r^-1 * (s*R - e*G)
    BigNum r_inv;
    BN_mod_inverse(r_inv, r, order, ctx);

    BIGNUM *e = BN_bin2bn(hash.data(), hash.size(), nullptr);

    Point sR{group};
    EC_POINT_mul(group, sR, nullptr, R, s, ctx);

    Point eG{group};
    EC_POINT_mul(group, eG, e, nullptr, nullptr, ctx);

    Point Q{group};
    EC_POINT_invert(group, eG, ctx);
    EC_POINT_add(group, Q, sR, eG, ctx);
    EC_POINT_mul(group, Q, nullptr, Q, r_inv, ctx);

    // Compare recovered public key with actual public key
    int match = (EC_POINT_cmp(group, Q, pub_key, ctx) == 0);

    // Cleanup
    // EC_POINT_free(R);
    // EC_POINT_free(sR);
    // EC_POINT_free(eG);
    // EC_POINT_free(Q);
    BN_free(x);
    BN_free(y);
    BN_free(y_squared);
    BN_free(tmp);
    // BN_free(r_inv);
    BN_free(e);

    if (match) {
      BN_free(order);  // XXX
      BN_free(p);      // XXX
      BN_CTX_free(ctx);
      return recovery_id;
    }
  }

  // BN_free(order);
  // BN_free(p);
  BN_CTX_free(ctx);

  // Default to 0 if recovery fails
  return 0;
}
}  // namespace

// === IMPLEMENTATION ===

Wallet::Wallet(std::string_view const &private_key) : key_{private_key}, address_{key_.derive_address()} {
}

Signature Wallet::sign_message(std::span<uint8_t const> const &hash) const {
  if (hash.size() != 32) {
    throw std::invalid_argument("Hash must be 32 bytes");
  }

  auto group = static_cast<EC_GROUP const *>(key_);
  auto priv_key = key_.get_private_key();

  // Generate deterministic k using RFC 6979
  BIGNUM *k = generateDeterministicK(priv_key, hash, group);

  // Get curve order
  BigNum order;
  EC_GROUP_get_order(group, order, nullptr);  // XXX

  BN_CTX *ctx = BN_CTX_new();

  // Calculate r = (k * G).x mod order
  Point kG{group};
  kG.multiply(group, k, nullptr, nullptr, ctx);

  BigNum r;
  BigNum y_coord;
  kG.get_affine_coordinates(group, r, y_coord, ctx);
  BN_mod(r, r, order, ctx);

  // Calculate s = k^-1 * (hash + r * priv_key) mod order
  BIGNUM *k_inv = BN_new();
  BN_mod_inverse(k_inv, k, order, ctx);

  BIGNUM *e = BN_bin2bn(hash.data(), hash.size(), nullptr);
  BigNum s;
  BigNum tmp;

  BN_mod_mul(tmp, r, priv_key, order, ctx);  // r * priv_key
  BN_mod_add(tmp, e, tmp, order, ctx);       // hash + r * priv_key
  BN_mod_mul(s, k_inv, tmp, order, ctx);     // k^-1 * (hash + r * priv_key)

  // Ensure s is in the lower half (ETH requirement for non-malleability)
  BigNum half_order;
  BN_rshift1(half_order, order);
  if (BN_cmp(s, half_order) > 0) {
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
  int recovery_id = calculateRecoveryId(key_, hash, sig);
  result.v = recovery_id + 27;  // Ethereum uses 27/28

  // Cleanup
  ECDSA_SIG_free(sig);
  // EC_POINT_free(kG);
  BN_free(k);
  // BN_free(order);
  // BN_free(r);
  // BN_free(s);
  // BN_free(y_coord);
  BN_free(k_inv);
  BN_free(e);
  // BN_free(tmp);
  BN_free(half_order);
  BN_CTX_free(ctx);

  return result;
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
