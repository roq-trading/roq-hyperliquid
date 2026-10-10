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

#include "roq/server/stream.hpp"

#include "roq/hyperliquid/gateway/shared.hpp"

#include "roq/hyperliquid/protocol/json/parser.hpp"

namespace roq {
namespace hyperliquid {
namespace gateway {

struct MarketData final : public Base<MarketData>,
                          public server::MarketDataStream,
                          public web::socket::Client::Handler,
                          public protocol::json::Parser::Handler {
  struct Handler {};

  MarketData(Handler &, io::Context &, uint16_t stream_id, Shared &, size_t index);

  // protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Trace<Start> const &) override;
  void operator()(Trace<Stop> const &) override;
  void operator()(Trace<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

  // server::MarketDataStream

  void subscribe(size_t start_from = 0) override;

 protected:
  // web::socket::Client::Handler

  void operator()(Trace<web::socket::Connected> const &) override;
  void operator()(Trace<web::socket::Disconnected> const &) override;
  void operator()(Trace<web::socket::Ready> const &) override;
  void operator()(Trace<web::socket::Close> const &) override;
  void operator()(Trace<web::socket::Latency> const &) override;
  void operator()(Trace<web::socket::Text> const &) override;
  void operator()(Trace<web::socket::Binary> const &) override;

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
  //
  void operator()(Trace<protocol::json::ActionError> const &) override;
  void operator()(Trace<protocol::json::ActionOrder> const &) override;
  void operator()(Trace<protocol::json::ActionCancel> const &) override;

  // helpers

  void get_spot_meta();

  void subscribe(std::span<Symbol const> const &symbols);
  void subscribe(std::string_view const &channel, std::span<Symbol const> const &symbols);

  void send_ping(std::chrono::nanoseconds now);

  void parse(std::string_view const &message);

 private:
  [[maybe_unused]] Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  size_t const index_;
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
    utils::metrics::Profile parse, pong, error, subscription_response, bbo, l2book, trades, active_asset_ctx, spot_meta;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
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
