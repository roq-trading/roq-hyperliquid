/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/hyperliquid/rest.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/hyperliquid/json/map.hpp"
#include "roq/hyperliquid/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {

// === TODO ===
// => use rate limiter / request queue
// => query instrument-info every N seconds

// === CONSTANTS ===

namespace {
auto const NAME = "rest"sv;

auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.rest.uri;
  auto config = web::rest::Client::Config{
      // connection
      .interface = {},
      .proxy = settings.rest.proxy,
      .uris = {&uri, 1},
      .host = settings.rest.host,
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = {},
      .disconnect_on_idle_timeout = {},
      .connection = web::http::Connection::KEEP_ALIVE,
      // request
      .allow_pipelining = true,
      .request_timeout = settings.rest.request_timeout,
      // response
      .suspend_on_retry_after = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .ping_frequency = settings.rest.ping_freq,
      .ping_path = settings.rest.ping_path,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::rest::Client::create(handler, context, config);
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};

auto create_rate_limiter(auto &settings) {
  return core::limit::RateLimiter{settings.request.limit, settings.request.limit_interval};
}
}  // namespace

// === IMPLEMENTATION ===

Rest::Rest(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_(shared.settings.misc.decode_buffer_size),
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .info = create_metrics(shared.settings, name_, "info"sv),
          .info_ack = create_metrics(shared.settings, name_, "info_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      shared_{shared}, download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }},
      rate_limiter{create_rate_limiter(shared.settings)} {
}

void Rest::operator()(Event<Start> const &) {
  (*connection_).start();
}

void Rest::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void Rest::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
}

void Rest::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.info, metrics::Type::PROFILE)
      .write(profile_.info_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

void Rest::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void Rest::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void Rest::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading()) {
    download_.reset();
  }
}

void Rest::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

uint32_t Rest::download(RestState state) {
  switch (state) {
    using enum RestState;
    case UNDEFINED:
      assert(false);
      break;
    case GET_INFO:
      get_info();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

// market info

void Rest::get_info() {
  profile_.info([&]() {
    auto body = R"({"type":"metaAndAssetCtxs"})"sv;
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = shared_.api.market_data.get_info,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = {},
        .body = body,
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_info_ack(event, sequence);
    };
    (*connection_)("info"sv, request, callback);
  });
}

void Rest::get_info_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = RestState::GET_INFO;
  profile_.info_ack([&]() {
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        // json::InstrumentInfo info{body, decode_buffer_};
        // Trace event_2{event, info};
        // (*this)(event_2);
        // XXX HANS NEW ??? create_trace_and_dispatch(*this, event, info)();
        download_.check(STATE);
      }
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      download_.retry(STATE);
    };
    process_response(event, handle_success, handle_error);
  });
}

void Rest::operator()(Trace<json::InstrumentInfo> const &event) {
  auto &[trace_info, instrument_info] = event;
  log::info<4>("instrument_info={}"sv, instrument_info);
}

// helpers

template <typename SuccessHandler, typename ErrorHandler>
void Rest::process_response(web::rest::Response const &response, SuccessHandler success_handler, ErrorHandler error_handler) {
  try {
    auto [status, category, body] = response.result();
    switch (category) {
      using enum web::http::Category;
      case SUCCESS:  // 2xx
        success_handler(body);
        break;
      case CLIENT_ERROR: {  // 4xx
        auto text = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, text);
        break;
      }
      case SERVER_ERROR: {  // 5xx
        auto text = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, text);
        break;
      }
      default:
        response.expect(web::http::Status::OK);  // throws
    }
  } catch (server::oms::Exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(e.origin, e.status, e.error, e.what());
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), e.what());
  } catch (std::exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, e.what());
  }
}

}  // namespace hyperliquid
}  // namespace roq
