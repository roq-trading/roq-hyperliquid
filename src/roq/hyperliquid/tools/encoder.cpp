/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/encoder.hpp"

#include <nlohmann/json.hpp>

#define MSGPACK_NO_BOOST
#include <msgpack.hpp>

#include "roq/logging.hpp"

#include "roq/utils/byte_order.hpp"
#include "roq/utils/common.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/hyperliquid/json/map.hpp"

#include "roq/hyperliquid/tools/conversions.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === HELPERS ===

namespace {
bool is_buy_helper(auto side) {
  switch (side) {
    using enum Side;
    case UNDEFINED:
      break;
    case BUY:
      return true;
    case SELL:
      return false;
  }
  log::fatal("Unexpected"sv);
}

auto order_type_helper_2(auto &order) {
  switch (order.order_type) {
    using enum OrderType;
    case UNDEFINED:
      break;
    case MARKET:
      break;
    case LIMIT: {
      auto tif = map(order.time_in_force).template get<json::TimeInForce>();
      return nlohmann::ordered_json{
          {"limit",
           {
               {"tif", tif.as_raw_text()},
           }},
      };
    }
  }
  log::fatal("Unexpected"sv);
}

void pack_json(auto &packer, auto &node) {
  if (node.is_null()) {
    packer.pack_nil();
  } else if (node.is_boolean()) {
    packer.pack(node.template get<bool>());
  } else if (node.is_number_integer()) {
    packer.pack(node.template get<int64_t>());
  } else if (node.is_number_unsigned()) {
    packer.pack(node.template get<uint64_t>());
  } else if (node.is_number_float()) {
    packer.pack(node.template get<double>());
  } else if (node.is_string()) {
    packer.pack(node.template get<std::string>());
  } else if (node.is_array()) {
    packer.pack_array(node.size());
    for (auto &item : node) {
      pack_json(packer, item);
    }
  } else if (node.is_object()) {
    packer.pack_map(node.size());
    for (auto &[key, value] : node.items()) {
      packer.pack(key);
      pack_json(packer, value);
    }
  }
}

auto pack_helper(auto &action, auto now_utc, auto expires_after_utc) {
  // XXX FIXME TODO std::stringstream => prefer to use a fixed buffer supporting write(const char*, size_t s)
  std::stringstream ss;
  msgpack::packer<std::stringstream> packer(ss);
  // action
  pack_json(packer, action);
  std::string msgpack_str = ss.str();
  auto len = std::size(msgpack_str);
  std::vector<std::byte> data(len);
  std::memcpy(std::data(data), std::data(msgpack_str), len);
  // nonce
  auto nonce = now_utc.count();
  auto nonce_2 = utils::host_to_little_endian(nonce);  // XXX FIXME TODO docs say big endian ???
  for (auto i = 0; i < 8; ++i) {
    auto tmp = static_cast<std::byte>((nonce_2 >> ((7 - i) * 8)) & 0xff);
    data.push_back(tmp);
  }
  // vault address (none)
  data.push_back(std::byte{0x00});  // required
  // expires-after
  if (expires_after_utc.count() && expires_after_utc != now_utc) {
    data.push_back(std::byte{0x00});  // required
    auto expires_after = expires_after_utc.count();
    auto expires_after_2 = utils::host_to_little_endian(expires_after);  // XXX FIXME TODO docs say big endian ???
    for (auto i = 0; i < 8; ++i) {
      auto tmp = static_cast<std::byte>((expires_after_2 >> ((7 - i) * 8)) & 0xff);
      data.push_back(tmp);
    }
  }
  return data;
}
}  // namespace

// === IMPLEMENTATION ===

std::pair<std::string, std::vector<std::byte>> Encoder::create_order(
    CreateOrder const &create_order,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::chrono::milliseconds now_utc,
    std::chrono::milliseconds expires_after_utc) {
  auto is_buy = is_buy_helper(order.side);
  auto reduce_only = false;  // XXX FIXME TODO
  auto cloid = fmt::format("0x{:0>32}"sv, request_id);
  auto order_type = order_type_helper_2(order);
  auto sz_decimals = utils::decimal_digits(order.quantity_precision.precision);
  auto is_spot = order.external_security_id >= 10000;
  auto price = Conversions::floatToWire(Conversions::roundPrice(create_order.price, sz_decimals, is_spot));
  auto size = Conversions::floatToWire(Conversions::roundSize(create_order.quantity, sz_decimals));
  auto action = [&]() {
    auto order_type = order_type_helper_2(order);
    auto order_2 = nlohmann::ordered_json{
        {"a", order.external_security_id},
        {"b", is_buy},
        {"p", price},
        {"s", size},
        {"r", reduce_only},
        {"t", order_type},
        {"c", cloid},
    };
    auto orders = nlohmann::ordered_json{
        order_2,
    };
    auto result = nlohmann::ordered_json{
        {"type", "order"},
        {"orders", orders},
        {"grouping", "na"},
    };
    return result;
  }();
  auto packed = pack_helper(action, now_utc, expires_after_utc);
  auto action_2 = action.dump();
  return {action_2, packed};
}
// {"action":{"type":"order","orders":[{"a":1,"b":true,"p":"1500","s":"1","r":false,"t":{"limit":{"tif":"Gtc"}},"c":"0x000300515099a47a0000010000000003"}],"grouping":"na"}

std::pair<std::string, std::vector<std::byte>> Encoder::modify_order(
    ModifyOrder const &modify_order,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds now_utc,
    std::chrono::milliseconds expires_after_utc) {
  throw server::oms::NotSupported{"not supported"sv};
  /*
  std::string coin{order.symbol};
  auto tmp = fmt::format("0x{:0>32}"sv, static_cast<std::string_view>(order.client_order_id));

  crypto::Cloid cloid{tmp};
  crypto::OidOrCloid oid{cloid};
  auto is_buy = is_buy_helper(order.side);
  auto reduce_only = true;  // XXX FIXME TODO
  auto order_type = order_type_helper(order);
  auto action =
      exchange.ROQ_modifyOrder(oid, coin, order.external_security_id, is_buy, modify_order.quantity, modify_order.price, order_type, reduce_only, cloid);
  auto packed = pack_helper(action, now_utc, expires_after_utc);
  auto action_2 = action.dump();
  return {action_2, packed};
  */
}

// {"action":{"type":"batchModify","modifies":[{"oid":"0x000300515427d70e00000100000000be","order":{"a":1,"b":true,"p":"1600","s":"1","r":true,"t":{"limit":{"tif":"Gtc"}},"c":"0x000300515427d70e00000100000000be"}}]}

std::pair<std::string, std::vector<std::byte>> Encoder::cancel_order(
    CancelOrder const &,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds now_utc,
    std::chrono::milliseconds expires_after_utc) {
  auto helper = [&](auto &action) -> std::pair<std::string, std::vector<std::byte>> {
    auto packed = pack_helper(action, now_utc, expires_after_utc);
    auto action_2 = action.dump();
    return {action_2, packed};
  };
  if (std::empty(order.external_order_id)) {
    auto cloid = fmt::format("0x{:0>32}"sv, static_cast<std::string_view>(order.client_order_id));
    auto action = [&]() {
      auto cancel = nlohmann::ordered_json{
          {"asset", order.external_security_id},
          {"cloid", cloid},
      };
      auto cancels = nlohmann::ordered_json{
          cancel,
      };
      auto result = nlohmann::ordered_json{
          {"type", "cancelByCloid"},
          {"cancels", cancels},
      };
      return result;
    }();
    return helper(action);
  } else {
    auto oid = utils::charconv::from_chars<int64_t>(order.external_order_id);
    auto action = [&]() {
      auto cancel = nlohmann::ordered_json{
          {"a", order.external_security_id},
          {"o", oid},
      };
      auto cancels = nlohmann::ordered_json{
          cancel,
      };
      auto result = nlohmann::ordered_json{
          {"type", "cancel"},
          {"cancels", cancels},
      };
      return result;
    }();
    return helper(action);
  }
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
