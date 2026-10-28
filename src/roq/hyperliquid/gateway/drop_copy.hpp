/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download_2.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/hyperliquid/gateway/account.hpp"
#include "roq/hyperliquid/gateway/request.hpp"
#include "roq/hyperliquid/gateway/shared.hpp"

#include "roq/hyperliquid/protocol/json/parser.hpp"

namespace roq {
namespace hyperliquid {
namespace gateway {

struct DropCopy final : public Base<DropCopy>, public server::Stream, public web::socket::Client::Handler, public protocol::json::Parser::Handler {
  struct Handler {};

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, Request &);

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

 protected:
  // web::socket::Client::Handler

  void operator()(Trace<web::socket::Connected> const &) override;
  void operator()(Trace<web::socket::Disconnected> const &) override;
  void operator()(Trace<web::socket::Ready> const &) override;
  void operator()(Trace<web::socket::Close> const &) override;
  void operator()(Trace<web::socket::Latency> const &) override;
  void operator()(Trace<web::socket::Text> const &) override;
  void operator()(Trace<web::socket::Binary> const &) override;

  // core::Download

  enum class State {
    UNDEFINED = 0,
    SUBSCRIBE_NOTIFICATION,
    SUBSCRIBE_USER_FUNDINGS,
    GET_SPOT_CLEARING_HOUSE_STATE,
    GET_CLEARING_HOUSE_STATE,
    SUBSCRIBE_ORDER_UPDATES,
    GET_OPEN_ORDERS,
    SUBSCRIBE_USER_FILLS,
    GET_USER_FILLS,
    SUBSCRIBE_USER_EVENTS,
    DONE,
  };

  int32_t download(Trace<State> const &);

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

  void subscribe(std::string_view const &type);

  void send_ping(std::chrono::nanoseconds now);

  void check_request_queue(TraceInfo const &);

  void parse(TraceInfo const &, std::string_view const &message);

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
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse, pong, error, subscription_response, user, user_fundings, user_fills, order_updates, notification;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  Request &request_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download2<State> download_;
  // ping
  std::chrono::nanoseconds next_ping_ = {};
};

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
