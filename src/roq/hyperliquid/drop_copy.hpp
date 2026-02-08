/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/io/web/uri.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/hyperliquid/account.hpp"
#include "roq/hyperliquid/shared.hpp"

#include "roq/hyperliquid/json/parser.hpp"

namespace roq {
namespace hyperliquid {

struct DropCopy final : public web::socket::Client::Handler, public json::Parser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<MarketStatus> const &, bool is_last) = 0;
    virtual void operator()(Trace<TopOfBook> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<TradeSummary> const &, bool is_last) = 0;
    virtual void operator()(Trace<StatisticsUpdate> const &, bool is_last) = 0;
  };

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  DropCopy(DropCopy const &) = delete;

  uint16_t stream_id() const { return stream_id_; }

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

 protected:
  // web::socket::Client::Handler

  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  void subscribe();
  void subscribe(std::string_view const &type);

  void send_ping(std::chrono::nanoseconds now);

  void parse(std::string_view const &message);

  // json::Parser::Handler

  void operator()(Trace<json::Pong> const &) override;
  void operator()(Trace<json::Error> const &) override;
  void operator()(Trace<json::SubscriptionResponse> const &) override;
  //
  void operator()(Trace<json::BBO> const &) override;
  void operator()(Trace<json::L2Book> const &) override;
  void operator()(Trace<json::Trades> const &) override;
  void operator()(Trace<json::ActiveAssetCtx> const &) override;
  //
  void operator()(Trace<json::SpotMeta> const &) override;
  //
  void operator()(Trace<json::User> const &) override;
  void operator()(Trace<json::UserFundings> const &) override;
  void operator()(Trace<json::UserFills> const &) override;
  void operator()(Trace<json::OrderUpdates> const &) override;

  // helpers

  void operator()(Trace<server::oms::OrderUpdate> const &, std::string_view const &client_order_id);

 private:
  Handler &handler_;
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
    utils::metrics::Profile parse, pong, error, subscription_response, user, user_fundings, user_fills, order_updates;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus status_ = {};
  // ping
  std::chrono::nanoseconds next_ping_ = {};
};

}  // namespace hyperliquid
}  // namespace roq
