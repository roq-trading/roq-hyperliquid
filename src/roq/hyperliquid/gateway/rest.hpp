/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download_2.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/server/stream.hpp"

#include "roq/hyperliquid/gateway/shared.hpp"

#include "roq/hyperliquid/protocol/json/get_meta_ack.hpp"
#include "roq/hyperliquid/protocol/json/get_perp_dexs_ack.hpp"
#include "roq/hyperliquid/protocol/json/get_spot_meta_ack.hpp"

namespace roq {
namespace hyperliquid {
namespace gateway {

struct Rest final : public Base<Rest>, public server::Stream, public web::rest::Client::Handler {
  struct SymbolsUpdate final {
    std::span<Symbol const> symbols;
  };

  struct Handler {
    virtual void operator()(SymbolsUpdate &) = 0;
  };

  Rest(Handler &, io::Context &context, uint16_t stream_id, Shared &);

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
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Connected> const &) override;
  void operator()(Trace<web::rest::Disconnected> const &) override;
  void operator()(Trace<web::rest::Latency> const &) override;

  // core::Download

  enum class State {
    UNDEFINED = 0,
    SPOT_META,
    PERP_DEXS,
    META,
    DONE,
  };

  int32_t download(Trace<State> const &);

  // spot-meta

  void get_spot_meta();
  void get_spot_meta_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::GetSpotMetaAck> const &);

  // perp-dexs

  void get_perp_dexs();
  void get_perp_dexs_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::GetPerpDexsAck> const &);

  // meta

  void get_meta(size_t index);
  void get_meta_ack(Trace<web::rest::Response> const &, uint32_t sequence, size_t index);
  void operator()(Trace<protocol::json::GetMetaAck> const &, size_t index);

  // helpers

  void process_response(Trace<web::rest::Response> const &, auto error_handler, auto success_handler);

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
    utils::metrics::Profile spot_meta, spot_meta_ack, perp_dexs, perp_dexs_ack, meta, meta_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download2<State> download_;
  // ...
  core::limit::RateLimiter rate_limiter;
};

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
