#include "roq/hyperliquid/crypto/exchange.hpp"

#include <cmath>

#include "roq/logging.hpp"

#include "roq/hyperliquid/crypto/constants.hpp"
#include "roq/hyperliquid/crypto/conversions.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace crypto {

Exchange::Exchange(
    Wallet &wallet,
    std::string const &base_url,
    Meta const *meta,
    std::string const &vault_address,
    std::string const &account_address,
    SpotMeta const *spot_meta,
    std::vector<std::string> const *perp_dexs,
    int timeout_ms)
    : wallet_{wallet}, base_url_{base_url}, vault_address_(vault_address), account_address_(account_address), expires_after_(std::nullopt) {
  log::warn("DEBUG account_address={}"sv, account_address_);
}

std::vector<uint8_t> Exchange::ROQ_actionHash(nlohmann::ordered_json const &action, std::chrono::milliseconds timestamp) {
  std::optional<std::string> vault_opt = vault_address_.empty() ? std::nullopt : std::optional<std::string>(vault_address_);
  return actionHash(action, vault_opt, timestamp.count(), expires_after_);
}

std::string Exchange::ROQ_sign(std::string_view const &action, std::vector<uint8_t> const &hash, std::chrono::milliseconds timestamp) {
  bool is_mainnet = (base_url_ == MAINNET_API_URL);
  std::optional<std::string> vault_opt = vault_address_.empty() ? std::nullopt : std::optional<std::string>(vault_address_);
  auto signature = ROQ_signL1Action(wallet_, hash, vault_opt, timestamp.count(), expires_after_, is_mainnet);
  return ROQ_postAction(action, signature, timestamp.count());
}

std::string Exchange::ROQ_postAction(std::string_view const &action, std::string const &signature, int64_t nonce) {
  auto result = fmt::format(
      R"({{)"
      R"("action":{},)"
      R"("nonce":{},)"
      R"("signature":{})"
      R"(}})"sv,
      action,
      nonce,
      signature);
  return result;
}

std::string Exchange::postAction(nlohmann::json const &action, Signature const &signature, int64_t nonce) {
  nlohmann::json payload = {
      {"action", action},
      {"nonce", nonce},
      {"signature", signature.toJson()},
  };
  // XXX FIXME TODO do we need vaultAddress ???
  // Add vault address if not a transfer action
  std::string action_type = action["type"];
  if (action_type != "usdClassTransfer" && action_type != "sendAsset") {
    if (!vault_address_.empty()) {
      payload["vaultAddress"] = vault_address_;
    } else {
      payload["vaultAddress"] = nullptr;
    }
  }

  // Add expires after if set
  if (expires_after_.has_value()) {
    payload["expiresAfter"] = expires_after_.value();
  } else {
    payload["expiresAfter"] = nullptr;
  }

  // return post("/exchange", payload);
  return payload.dump();
}

double Exchange::slippagePrice(std::string const &name, bool is_buy, double slippage, std::optional<double> px) {
  std::string coin = nameToCoin(name);

  // Get mid price if not provided
  if (!px.has_value()) {
    // auto mids = info_.allMids("");
    // px = std::stod(mids[coin].get<std::string>());
  }

  int asset = coin_to_asset(coin);
  bool is_spot = asset >= 10000;
  int sz_decimals = asset_to_sz_decimals(asset);

  // Calculate slippage
  double price = px.value();
  price *= is_buy ? (1.0 + slippage) : (1.0 - slippage);

  // Round to tick size (5 significant figures and MAX_DECIMALS - szDecimals)
  return roundPrice(price, sz_decimals, is_spot);
}

void Exchange::setExpiresAfter(std::optional<int64_t> expires_after) {
  expires_after_ = expires_after;
}

std::string Exchange::order(
    std::string const &coin,
    bool is_buy,
    double sz,
    double limit_px,
    OrderType const &order_type,
    bool reduce_only,
    std::optional<Cloid> const &cloid,
    std::optional<BuilderInfo> const &builder) {
  // Get asset info for rounding
  int asset = nameToAsset(coin);
  int sz_decimals = asset_to_sz_decimals(asset);
  bool is_spot = asset >= 10000;

  // Round price and size to tick/lot size
  double rounded_px = roundPrice(limit_px, sz_decimals, is_spot);
  double rounded_sz = roundSize(sz, sz_decimals);

  OrderRequest order_req;
  order_req.coin = coin;
  order_req.is_buy = is_buy;
  order_req.sz = rounded_sz;
  order_req.limit_px = rounded_px;
  order_req.order_type = order_type;
  order_req.reduce_only = reduce_only;
  order_req.cloid = cloid;

  return bulkOrders({order_req}, builder);
}

nlohmann::ordered_json Exchange::ROQ_order(
    std::string const &coin,
    int32_t external_security_id,
    int8_t quantity_decimals,
    int8_t price_decimals,
    bool is_buy,
    double sz,
    double limit_px,
    OrderType const &order_type,
    bool reduce_only,
    std::optional<Cloid> const &cloid,
    std::optional<BuilderInfo> const &builder) {
  // Get asset info for rounding
  int asset = external_security_id;
  int sz_decimals = quantity_decimals;  // asset_to_sz_decimals(asset);
  bool is_spot = asset >= 10000;

  // Round price and size to tick/lot size
  double rounded_px = roundPrice(limit_px, sz_decimals, is_spot);
  double rounded_sz = roundSize(sz, sz_decimals);

  OrderRequest order_req;
  order_req.coin = coin;
  order_req.is_buy = is_buy;
  order_req.sz = rounded_sz;
  order_req.limit_px = rounded_px;
  order_req.order_type = order_type;
  order_req.reduce_only = reduce_only;
  order_req.cloid = cloid;

  order_req.ROQ_external_security_id = asset;           // XXX
  order_req.ROQ_quantity_decimals = quantity_decimals;  // XXX
  order_req.ROQ_price_decimals = price_decimals;        // XXX

  return ROQ_bulkOrders({order_req}, builder);
}

std::string Exchange::bulkOrders(std::vector<OrderRequest> const &orders, std::optional<BuilderInfo> const &builder, std::string const &grouping) {
  std::vector<OrderWire> order_wires;
  for (auto const &order : orders) {
    int asset = order.ROQ_external_security_id;     // nameToAsset(order.coin);
    int sz_decimals = order.ROQ_quantity_decimals;  // asset_to_sz_decimals(asset);
    bool is_spot = asset >= 10000;

    // Round price and size to tick/lot size
    OrderRequest rounded_order = order;
    rounded_order.limit_px = roundPrice(order.limit_px, sz_decimals, is_spot);
    rounded_order.sz = roundSize(order.sz, sz_decimals);

    order_wires.push_back(orderRequestToOrderWire(rounded_order, asset));
  }

  int64_t timestamp = getTimestampMs();

  // Create order action
  auto action = orderWiresToOrderAction(order_wires, builder, grouping);

  // Determine if mainnet
  bool is_mainnet = (base_url_ == MAINNET_API_URL);

  // Sign action
  std::optional<std::string> vault_opt = vault_address_.empty() ? std::nullopt : std::optional<std::string>(vault_address_);
  auto signature = signL1Action(wallet_, action, vault_opt, timestamp, expires_after_, is_mainnet);

  return postAction(action, signature, timestamp);
}

nlohmann::ordered_json Exchange::ROQ_bulkOrders(
    std::vector<OrderRequest> const &orders, std::optional<BuilderInfo> const &builder, std::string const &grouping) {
  std::vector<OrderWire> order_wires;
  for (auto const &order : orders) {
    int asset = order.ROQ_external_security_id;     // nameToAsset(order.coin);
    int sz_decimals = order.ROQ_quantity_decimals;  // asset_to_sz_decimals(asset);
    bool is_spot = asset >= 10000;

    // Round price and size to tick/lot size
    OrderRequest rounded_order = order;
    rounded_order.limit_px = roundPrice(order.limit_px, sz_decimals, is_spot);
    rounded_order.sz = roundSize(order.sz, sz_decimals);

    order_wires.push_back(orderRequestToOrderWire(rounded_order, asset));
  }

  return orderWiresToOrderAction(order_wires, builder, grouping);
}

std::string Exchange::marketOpen(
    std::string const &coin,
    bool is_buy,
    double sz,
    std::optional<double> px,
    double slippage,
    std::optional<Cloid> const &cloid,
    std::optional<BuilderInfo> const &builder) {
  double price = slippagePrice(coin, is_buy, slippage, px);

  OrderType order_type;
  order_type.limit = LimitOrderType{"Ioc"};  // Immediate or cancel

  return order(coin, is_buy, sz, price, order_type, false, cloid, builder);
}

std::string Exchange::marketClose(
    std::string const &coin,
    std::optional<double> sz,
    std::optional<double> px,
    double slippage,
    std::optional<Cloid> const &cloid,
    std::optional<BuilderInfo> const &builder) {
  // Get user state to determine position size and direction
  std::string address{wallet_.address()};
  auto user_state = userState(address);

  // Find position
  double position_sz = 0.0;
  bool found = false;
  for (auto const &asset_pos : user_state["assetPositions"]) {
    auto pos = asset_pos["position"];
    if (pos["coin"] == coin) {
      position_sz = std::stod(pos["szi"].get<std::string>());
      found = true;
      break;
    }
  }

  if (!found || std::abs(position_sz) < 1e-8) {
    throw std::runtime_error("No position to close for " + coin);
  }

  // Determine close size and direction
  double close_sz = sz.has_value() ? sz.value() : std::abs(position_sz);
  bool is_buy = position_sz < 0;  // Buy to close short, sell to close long

  return marketOpen(coin, is_buy, close_sz, px, slippage, cloid, builder);
}

std::string Exchange::cancel(std::string const &coin, int64_t oid) {
  CancelRequest cancel_req;
  cancel_req.coin = coin;
  cancel_req.oid = oid;
  cancel_req.ROQ_external_security_id = nameToAsset(coin);
  return bulkCancel({cancel_req});
}

nlohmann::ordered_json Exchange::ROQ_cancel(std::string const &coin, int32_t external_security_id, int64_t oid) {
  CancelRequest cancel_req;
  cancel_req.coin = coin;
  cancel_req.oid = oid;
  cancel_req.ROQ_external_security_id = external_security_id;
  return ROQ_bulkCancel({cancel_req});
}

std::string Exchange::cancelByCloid(std::string const &coin, Cloid const &cloid) {
  CancelByCloidRequest cancel_req{coin, cloid};
  cancel_req.ROQ_external_security_id = nameToAsset(coin);
  return bulkCancelByCloid({cancel_req});
}

nlohmann::ordered_json Exchange::ROQ_cancelByCloid(std::string const &coin, int32_t external_security_id, Cloid const &cloid) {
  CancelByCloidRequest cancel_req{coin, cloid};
  cancel_req.ROQ_external_security_id = external_security_id;
  return ROQ_bulkCancelByCloid({cancel_req});
}

std::string Exchange::bulkCancel(std::vector<CancelRequest> const &cancels) {
  nlohmann::ordered_json cancels_array = nlohmann::ordered_json::array();
  for (auto const &cancel : cancels) {
    int asset = cancel.ROQ_external_security_id;
    nlohmann::ordered_json cancel_obj;
    cancel_obj["a"] = asset;
    cancel_obj["o"] = cancel.oid;
    cancels_array.push_back(cancel_obj);
  }

  nlohmann::ordered_json action;
  action["type"] = "cancel";
  action["cancels"] = cancels_array;

  int64_t timestamp = getTimestampMs();
  bool is_mainnet = (base_url_ == MAINNET_API_URL);

  std::optional<std::string> vault_opt = vault_address_.empty() ? std::nullopt : std::optional<std::string>(vault_address_);
  auto signature = signL1Action(wallet_, action, vault_opt, timestamp, expires_after_, is_mainnet);

  return postAction(action, signature, timestamp);
}

nlohmann::ordered_json Exchange::ROQ_bulkCancel(std::vector<CancelRequest> const &cancels) {
  nlohmann::ordered_json cancels_array = nlohmann::ordered_json::array();
  for (auto const &cancel : cancels) {
    int asset = cancel.ROQ_external_security_id;
    nlohmann::ordered_json cancel_obj;
    cancel_obj["a"] = asset;
    cancel_obj["o"] = cancel.oid;
    cancels_array.push_back(cancel_obj);
  }

  nlohmann::ordered_json action;
  action["type"] = "cancel";
  action["cancels"] = cancels_array;

  return action;
}

std::string Exchange::bulkCancelByCloid(std::vector<CancelByCloidRequest> const &cancels) {
  nlohmann::ordered_json cancels_array = nlohmann::ordered_json::array();
  for (auto const &cancel : cancels) {
    int asset = cancel.ROQ_external_security_id;
    nlohmann::ordered_json cancel_obj;
    cancel_obj["asset"] = asset;
    cancel_obj["cloid"] = cancel.cloid.toRaw();
    cancels_array.push_back(cancel_obj);
  }

  nlohmann::ordered_json action;
  action["type"] = "cancelByCloid";
  action["cancels"] = cancels_array;

  int64_t timestamp = getTimestampMs();
  bool is_mainnet = (base_url_ == MAINNET_API_URL);

  std::optional<std::string> vault_opt = vault_address_.empty() ? std::nullopt : std::optional<std::string>(vault_address_);
  auto signature = signL1Action(wallet_, action, vault_opt, timestamp, expires_after_, is_mainnet);

  return postAction(action, signature, timestamp);
}

nlohmann::ordered_json Exchange::ROQ_bulkCancelByCloid(std::vector<CancelByCloidRequest> const &cancels) {
  nlohmann::ordered_json cancels_array = nlohmann::ordered_json::array();
  for (auto const &cancel : cancels) {
    int asset = cancel.ROQ_external_security_id;
    nlohmann::ordered_json cancel_obj;
    cancel_obj["asset"] = asset;
    cancel_obj["cloid"] = cancel.cloid.toRaw();
    cancels_array.push_back(cancel_obj);
  }

  nlohmann::ordered_json action;
  action["type"] = "cancelByCloid";
  action["cancels"] = cancels_array;

  return action;
}

std::string Exchange::modifyOrder(
    OidOrCloid const &oid,
    std::string const &coin,
    bool is_buy,
    double sz,
    double limit_px,
    OrderType const &order_type,
    bool reduce_only,
    std::optional<Cloid> const &cloid) {
  // Get asset info for rounding
  int asset = nameToAsset(coin);
  int sz_decimals = asset_to_sz_decimals(asset);
  bool is_spot = asset >= 10000;

  // Round price and size to tick/lot size
  double rounded_px = roundPrice(limit_px, sz_decimals, is_spot);
  double rounded_sz = roundSize(sz, sz_decimals);

  ModifyRequest modify_req;
  modify_req.oid = oid;
  modify_req.order.coin = coin;
  modify_req.order.is_buy = is_buy;
  modify_req.order.sz = rounded_sz;
  modify_req.order.limit_px = rounded_px;
  modify_req.order.order_type = order_type;
  modify_req.order.reduce_only = reduce_only;
  modify_req.order.cloid = cloid;

  return bulkModifyOrders({modify_req});
}

nlohmann::ordered_json Exchange::ROQ_modifyOrder(
    OidOrCloid const &oid,
    std::string const &coin,
    int32_t external_security_id,
    bool is_buy,
    double sz,
    double limit_px,
    OrderType const &order_type,
    bool reduce_only,
    std::optional<Cloid> const &cloid) {
  // Get asset info for rounding
  int asset = external_security_id;
  int sz_decimals = asset_to_sz_decimals(asset);
  bool is_spot = asset >= 10000;

  // Round price and size to tick/lot size
  double rounded_px = roundPrice(limit_px, sz_decimals, is_spot);
  double rounded_sz = roundSize(sz, sz_decimals);

  ModifyRequest modify_req;
  modify_req.oid = oid;
  modify_req.order.coin = coin;
  modify_req.order.is_buy = is_buy;
  modify_req.order.sz = rounded_sz;
  modify_req.order.limit_px = rounded_px;
  modify_req.order.order_type = order_type;
  modify_req.order.reduce_only = reduce_only;
  modify_req.order.cloid = cloid;
  //
  modify_req.order.ROQ_external_security_id = external_security_id;

  return ROQ_bulkModifyOrders({modify_req});
}

std::string Exchange::bulkModifyOrders(std::vector<ModifyRequest> const &modifies) {
  nlohmann::ordered_json modifies_array = nlohmann::ordered_json::array();
  for (auto const &modify : modifies) {
    int asset = modify.order.ROQ_external_security_id;  // XXX
    int sz_decimals = asset_to_sz_decimals(asset);
    bool is_spot = asset >= 10000;

    // Round price and size to tick/lot size
    OrderRequest rounded_order = modify.order;
    rounded_order.limit_px = roundPrice(modify.order.limit_px, sz_decimals, is_spot);
    rounded_order.sz = roundSize(modify.order.sz, sz_decimals);

    OrderWire wire = orderRequestToOrderWire(rounded_order, asset);

    nlohmann::ordered_json modify_wire;
    if (std::holds_alternative<int64_t>(modify.oid)) {
      modify_wire["oid"] = std::get<int64_t>(modify.oid);
    } else {
      modify_wire["oid"] = std::get<Cloid>(modify.oid).toRaw();
    }
    modify_wire["order"] = wire.toJson();

    modifies_array.push_back(modify_wire);
  }

  nlohmann::ordered_json action;
  action["type"] = "batchModify";
  action["modifies"] = modifies_array;

  int64_t timestamp = getTimestampMs();
  bool is_mainnet = (base_url_ == MAINNET_API_URL);

  std::optional<std::string> vault_opt = vault_address_.empty() ? std::nullopt : std::optional<std::string>(vault_address_);
  auto signature = signL1Action(wallet_, action, vault_opt, timestamp, expires_after_, is_mainnet);

  return postAction(action, signature, timestamp);
}

nlohmann::ordered_json Exchange::ROQ_bulkModifyOrders(std::vector<ModifyRequest> const &modifies) {
  nlohmann::ordered_json modifies_array = nlohmann::ordered_json::array();
  for (auto const &modify : modifies) {
    int asset = modify.order.ROQ_external_security_id;  // XXX
    int sz_decimals = asset_to_sz_decimals(asset);
    bool is_spot = asset >= 10000;

    // Round price and size to tick/lot size
    OrderRequest rounded_order = modify.order;
    rounded_order.limit_px = roundPrice(modify.order.limit_px, sz_decimals, is_spot);
    rounded_order.sz = roundSize(modify.order.sz, sz_decimals);

    OrderWire wire = orderRequestToOrderWire(rounded_order, asset);

    nlohmann::ordered_json modify_wire;
    if (std::holds_alternative<int64_t>(modify.oid)) {
      modify_wire["oid"] = std::get<int64_t>(modify.oid);
    } else {
      modify_wire["oid"] = std::get<Cloid>(modify.oid).toRaw();
    }
    modify_wire["order"] = wire.toJson();

    modifies_array.push_back(modify_wire);
  }

  nlohmann::ordered_json action;
  action["type"] = "batchModify";
  action["modifies"] = modifies_array;

  return action;
}

std::string Exchange::usdTransfer(double amount, std::string const &destination) {
  nlohmann::json action = {{"type", "usdSend"}, {"destination", destination}, {"amount", floatToWire(amount)}, {"time", getTimestampMs()}};

  std::vector<EIP712Type> payload_types = {{"hyperliquidChain", "string"}, {"destination", "string"}, {"amount", "string"}, {"time", "uint64"}};

  bool is_mainnet = (base_url_ == MAINNET_API_URL);
  auto signature = signUserSignedAction(wallet_, action, payload_types, "HyperliquidTransaction:UsdSend", is_mainnet);

  return postAction(action, signature, action["time"]);
}

std::string Exchange::spotTransfer(double amount, std::string const &destination, std::string const &token) {
  nlohmann::json action = {{"type", "spotSend"}, {"destination", destination}, {"token", token}, {"amount", floatToWire(amount)}, {"time", getTimestampMs()}};

  std::vector<EIP712Type> payload_types = {
      {"hyperliquidChain", "string"}, {"destination", "string"}, {"token", "string"}, {"amount", "string"}, {"time", "uint64"}};

  bool is_mainnet = (base_url_ == MAINNET_API_URL);
  auto signature = signUserSignedAction(wallet_, action, payload_types, "HyperliquidTransaction:SpotSend", is_mainnet);

  return postAction(action, signature, action["time"]);
}

std::string Exchange::updateLeverage(int leverage, std::string const &coin, bool is_cross) {
  int asset = nameToAsset(coin);

  nlohmann::ordered_json leverage_obj;
  if (is_cross) {
    leverage_obj["type"] = "cross";
    leverage_obj["value"] = leverage;
  } else {
    leverage_obj["type"] = "isolated";
    leverage_obj["value"] = leverage;
  }

  nlohmann::ordered_json action;
  action["type"] = "updateLeverage";
  action["asset"] = asset;
  action["isCross"] = is_cross;
  action["leverage"] = leverage;

  int64_t timestamp = getTimestampMs();
  bool is_mainnet = (base_url_ == MAINNET_API_URL);

  std::optional<std::string> vault_opt = vault_address_.empty() ? std::nullopt : std::optional<std::string>(vault_address_);
  auto signature = signL1Action(wallet_, action, vault_opt, timestamp, expires_after_, is_mainnet);

  return postAction(action, signature, timestamp);
}

std::string Exchange::scheduleCancel(std::optional<int64_t> time) {
  int64_t timestamp = getTimestampMs();

  nlohmann::ordered_json action;
  action["type"] = "scheduleCancel";
  if (time.has_value()) {
    action["time"] = time.value();
  }

  bool is_mainnet = (base_url_ == MAINNET_API_URL);

  std::optional<std::string> vault_opt = vault_address_.empty() ? std::nullopt : std::optional<std::string>(vault_address_);
  auto signature = signL1Action(wallet_, action, vault_opt, timestamp, expires_after_, is_mainnet);

  return postAction(action, signature, timestamp);
}

std::string Exchange::queryOrderByCloid(std::string const &user, Cloid const &cloid) {
  // return info_.queryOrderByCloid(user, cloid);
  return {};
}

// XXX info

int Exchange::nameToAsset(std::string const &name) const {
  return {};
}

std::string const &Exchange::nameToCoin(std::string const &name) const {
  static std::string const result;
  return result;
}

nlohmann::json Exchange::userState(std::string const &address, std::string const &dex) {
  return {};
}

nlohmann::json Exchange::queryOrderByOid(std::string const &user, int64_t oid) {
  return {};
}

int Exchange::asset_to_sz_decimals(int asset) {
  return {};
}

int Exchange::coin_to_asset(std::string const &coin) {
  return {};
}

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
