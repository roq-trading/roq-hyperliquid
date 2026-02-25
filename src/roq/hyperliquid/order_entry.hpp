/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/utils/container.hpp"

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/hyperliquid/account.hpp"
#include "roq/hyperliquid/order_entry_state.hpp"
#include "roq/hyperliquid/shared.hpp"

#include "roq/hyperliquid/json/get_clearing_house_state_ack.hpp"
#include "roq/hyperliquid/json/get_open_orders_ack.hpp"
#include "roq/hyperliquid/json/get_spot_clearing_house_state_ack.hpp"
#include "roq/hyperliquid/json/get_user_fills_ack.hpp"

#include "roq/hyperliquid/json/cancel_order_ack.hpp"
#include "roq/hyperliquid/json/create_order_ack.hpp"
#include "roq/hyperliquid/json/modify_order_ack.hpp"

namespace roq {
namespace hyperliquid {

struct OrderEntry final : public web::rest::Client::Handler {
  struct SymbolsUpdate final {
    std::span<Symbol const> symbols;
  };

  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
  };

  OrderEntry(Handler &, io::Context &context, uint16_t stream_id, Account &, Shared &);

  OrderEntry(OrderEntry const &) = delete;

  bool ready() const { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id);
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  uint32_t download(OrderEntryState);

  // clearing-house-state

  void get_clearing_house_state(size_t index);
  void get_clearing_house_state_ack(Trace<web::rest::Response> const &, uint32_t sequence, size_t index);
  void operator()(Trace<json::GetClearingHouseStateAck> const &, size_t index);

  // spot-clearing-house-state

  void get_spot_clearing_house_state();
  void get_spot_clearing_house_state_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::GetSpotClearingHouseStateAck> const &);

  // open-orders

  void get_open_orders(size_t index);
  void get_open_orders_ack(Trace<web::rest::Response> const &, uint32_t sequence, size_t index);
  void operator()(Trace<json::GetOpenOrdersAck> const &, size_t index);

  // user-fills

  void get_user_fills(size_t index);
  void get_user_fills_ack(Trace<web::rest::Response> const &, uint32_t sequence, size_t index);
  void operator()(Trace<json::GetUserFillsAck> const &, size_t index);

  // create-order

  void create_order(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id);
  void create_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::CreateOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // modify-order

  void modify_order(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void modify_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::ModifyOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-order

  void cancel_order(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::CancelOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // helpers

  void process_response(web::rest::Response const &, auto error_handler, auto success_handler);

  void operator()(Trace<server::oms::OrderUpdate> const &, std::string_view const &client_order_id);

  template <typename... Args>
  void operator()(Trace<server::oms::Response> const &, uint8_t user_id, uint64_t order_id, Args &&...args);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // connection
  std::unique_ptr<web::rest::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile spot_clearing_house_state, spot_clearing_house_state_ack, clearing_house_state, clearing_house_state_ack, open_orders,
        open_orders_ack, user_fills, user_fills_ack, create_order, create_order_ack, modify_order, modify_order_ack, cancel_order, cancel_order_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download<OrderEntryState> download_;
  // ...
  core::limit::RateLimiter rate_limiter;
};

}  // namespace hyperliquid
}  // namespace roq
