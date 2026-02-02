/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/crypto/wallet.hpp"

#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <iomanip>
#include <sstream>
#include <vector>

#include "roq/exceptions.hpp"

#include "roq/hyperliquid/crypto/ecdsa.hpp"

#define MSGPACK_NO_BOOST
#include <msgpack.hpp>

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace crypto {

// === HELPERS ===

namespace {
auto create(auto &private_key_hex) {
  std::string tmp{private_key_hex};
  return crypto::createKeyFromPrivate(tmp);
}

void freeKey(void *ec_key_ptr) {
  if (ec_key_ptr) {
    EC_KEY_free(static_cast<EC_KEY *>(ec_key_ptr));
  }
}

}  // namespace

// === IMPLEMENTATION ===

Wallet::Wallet(void *ec_key) : ec_key_(ec_key) {
  address_ = crypto::deriveAddress(ec_key_);
}

Wallet::~Wallet() {
  freeKey(ec_key_);
}

/*
std::shared_ptr<Wallet> Wallet::fromPrivateKey(std::string const &private_key_hex) {
  void *ec_key = crypto::createKeyFromPrivate(private_key_hex);
  return std::shared_ptr<Wallet>(new Wallet(ec_key));
}
*/

Wallet::Wallet(std::string_view const &private_key_hex) : ec_key_{create(private_key_hex)}, address_{crypto::deriveAddress(ec_key_)} {
}

std::string Wallet::address() const {
  return address_;
}

Signature Wallet::signMessage(std::vector<uint8_t> const &message_hash) const {
  return signHash(ec_key_, message_hash);
}

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
