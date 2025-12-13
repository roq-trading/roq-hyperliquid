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

#include "roq/hyperliquid/rest_state.hpp"
#include "roq/hyperliquid/shared.hpp"

#include "roq/hyperliquid/json/meta.hpp"
#include "roq/hyperliquid/json/perp_dexs.hpp"
#include "roq/hyperliquid/json/spot_meta.hpp"

namespace roq {
namespace hyperliquid {

struct Rest final : public web::rest::Client::Handler {
  struct SymbolsUpdate final {
    std::vector<Symbol> &symbols;
  };

  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<ReferenceData> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketStatus> const &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(SymbolsUpdate &) = 0;
  };

  Rest(Handler &, io::Context &context, uint16_t stream_id, Shared &);

  Rest(Rest const &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

 protected:
  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;

  void operator()(ConnectionStatus);

  uint32_t download(RestState);

  void get_spot_meta();
  void get_spot_meta_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::SpotMeta> const &);

  void get_perp_dexs();
  void get_perp_dexs_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::PerpDexs> const &);

  void get_meta();
  void get_meta_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::Meta> const &);

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
  utils::unordered_set<std::string> symbols_;
  // state
  ConnectionStatus status_ = {};
  core::Download<RestState> download_;
  // ...
  core::limit::RateLimiter rate_limiter;
};

}  // namespace hyperliquid
}  // namespace roq
