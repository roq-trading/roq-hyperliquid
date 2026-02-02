#include "roq/hyperliquid/crypto/keccak.hpp"

#include <openssl/evp.h>

#include <stdexcept>

#include "roq/hyperliquid/crypto/conversions.hpp"

namespace roq {
namespace hyperliquid {
namespace crypto {

std::vector<uint8_t> keccak256(uint8_t const *data, size_t len) {
  std::vector<uint8_t> hash(32);

  // OpenSSL 3.0+ uses EVP interface for Keccak
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (!ctx) {
    throw std::runtime_error("Failed to create EVP_MD_CTX");
  }

  const EVP_MD *md = EVP_MD_fetch(nullptr, "KECCAK-256", nullptr);
  if (!md) {
    EVP_MD_CTX_free(ctx);
    throw std::runtime_error("KECCAK-256 not available in OpenSSL");
  }

  if (EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
    EVP_MD_free(const_cast<EVP_MD *>(md));
    EVP_MD_CTX_free(ctx);
    throw std::runtime_error("EVP_DigestInit_ex failed");
  }

  if (EVP_DigestUpdate(ctx, data, len) != 1) {
    EVP_MD_free(const_cast<EVP_MD *>(md));
    EVP_MD_CTX_free(ctx);
    throw std::runtime_error("EVP_DigestUpdate failed");
  }

  unsigned int hash_len = 0;
  if (EVP_DigestFinal_ex(ctx, hash.data(), &hash_len) != 1) {
    EVP_MD_free(const_cast<EVP_MD *>(md));
    EVP_MD_CTX_free(ctx);
    throw std::runtime_error("EVP_DigestFinal_ex failed");
  }

  EVP_MD_free(const_cast<EVP_MD *>(md));
  EVP_MD_CTX_free(ctx);

  if (hash_len != 32) {
    throw std::runtime_error("Unexpected hash length");
  }

  return hash;
}

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
