/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/hyperliquid/gateway/account.hpp"
#include "roq/hyperliquid/gateway/shared.hpp"

#include "roq/hyperliquid/protocol/json/parser.hpp"

namespace roq {
namespace hyperliquid {
namespace gateway {

struct WebSocket final : public web::socket::Client::Handler, public protocol::json::Parser::Handler {
  struct Handler {};

  WebSocket(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  WebSocket(WebSocket const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  void subscribe(size_t start_from = 0);

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
  // web::socket::Client::Handler

  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

  // helpers

  uint16_t stream_id() const { return stream_id_; }

  bool ready() const { return connection_status_ == ConnectionStatus::READY; }

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  void send_ping(std::chrono::nanoseconds now);

  void parse(std::string_view const &message);

  // protocol::json::Parser::Handler

  void operator()(Trace<protocol::json::Pong> const &) override;
  void operator()(Trace<protocol::json::Error> const &) override;
  void operator()(Trace<protocol::json::SubscriptionResponse> const &) override;
  //
  void operator()(Trace<protocol::json::BBO> const &) override;
  void operator()(Trace<protocol::json::L2Book> const &) override;
  void operator()(Trace<protocol::json::Trades> const &) override;
  void operator()(Trace<protocol::json::ActiveAssetCtx> const &) override;
  //
  void operator()(Trace<protocol::json::SpotMeta> const &) override;
  //
  void operator()(Trace<protocol::json::User> const &) override;
  void operator()(Trace<protocol::json::UserFundings> const &) override;
  void operator()(Trace<protocol::json::UserFills> const &) override;
  void operator()(Trace<protocol::json::OrderUpdates> const &) override;
  void operator()(Trace<protocol::json::Notification> const &) override;

 private:
  [[maybe_unused]] Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  std::chrono::nanoseconds const ping_frequency_;
  // web socket
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // session
  uint64_t request_id_ = {};
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse, pong, error, subscription_response;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  // ping
  std::chrono::nanoseconds next_ping_ = {};
};

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
