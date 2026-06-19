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

#include "roq/hyperliquid/gateway/shared.hpp"

#include "roq/hyperliquid/protocol/json/get_meta_ack.hpp"
#include "roq/hyperliquid/protocol/json/get_perp_dexs_ack.hpp"
#include "roq/hyperliquid/protocol/json/get_spot_meta_ack.hpp"

namespace roq {
namespace hyperliquid {
namespace gateway {

struct Rest final : public web::rest::Client::Handler {
  struct SymbolsUpdate final {
    std::span<Symbol const> symbols;
  };

  struct Handler {
    virtual void operator()(SymbolsUpdate &) = 0;
  };

  Rest(Handler &, io::Context &context, uint16_t stream_id, Shared &);

  Rest(Rest const &) = delete;

  bool ready() const { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  enum class State {
    UNDEFINED = 0,
    SPOT_META,
    PERP_DEXS,
    META,
    DONE,
  };

  uint32_t download(State);

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

  void process_response(web::rest::Response const &, auto error_handler, auto success_handler);

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
  core::Download<State> download_;
  // ...
  core::limit::RateLimiter rate_limiter;
};

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
