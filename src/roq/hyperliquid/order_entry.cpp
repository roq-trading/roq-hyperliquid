/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/order_entry.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/hyperliquid/json/map.hpp"
#include "roq/hyperliquid/json/utils.hpp"

// #define MSGPACK_NO_BOOST
// #include <msgpack.hpp>

using namespace std::literals;

namespace roq {
namespace hyperliquid {

// === CONSTANTS ===

namespace {
auto const NAME = "rest"sv;

auto const SUPPORTS = Mask{
    SupportType::ORDER,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
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

OrderEntry::OrderEntry(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .clearing_house_state = create_metrics(shared.settings, name_, "clearing_house_state"sv),
          .clearing_house_state_ack = create_metrics(shared.settings, name_, "clearing_house_state_ack"sv),
          .spot_clearing_house_state = create_metrics(shared.settings, name_, "spot_clearing_house_state"sv),
          .spot_clearing_house_state_ack = create_metrics(shared.settings, name_, "spot_clearing_house_state_ack"sv),
          .open_orders = create_metrics(shared.settings, name_, "open_orders"sv),
          .open_orders_ack = create_metrics(shared.settings, name_, "open_orders_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      account_{account}, shared_{shared}, download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }},
      rate_limiter{create_rate_limiter(shared.settings)} {
}

void OrderEntry::operator()(Event<Start> const &) {
  (*connection_).start();
}

void OrderEntry::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void OrderEntry::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
}

void OrderEntry::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.clearing_house_state, metrics::Type::PROFILE)
      .write(profile_.clearing_house_state_ack, metrics::Type::PROFILE)
      .write(profile_.spot_clearing_house_state, metrics::Type::PROFILE)
      .write(profile_.spot_clearing_house_state_ack, metrics::Type::PROFILE)
      .write(profile_.open_orders, metrics::Type::PROFILE)
      .write(profile_.open_orders_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

uint16_t OrderEntry::operator()(Event<CreateOrder> const &, server::oms::Order const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &,
    server::oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &,
    server::oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t OrderEntry::operator()(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

void OrderEntry::operator()(ConnectionStatus status) {
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

// web::rest::Client::Handler

void OrderEntry::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading()) {
    download_.reset();
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    using enum OrderEntryState;
    case UNDEFINED:
      assert(false);
      break;
    case CLEARING_HOUSE_STATE:
      get_clearing_house_state();
      return 1;
    case SPOT_CLEARING_HOUSE_STATE:
      get_spot_clearing_house_state();
      return 1;
    case OPEN_ORDERS:
      get_open_orders();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

// clearing-house-state

void OrderEntry::get_clearing_house_state() {
  profile_.clearing_house_state([&]() {
    auto body = fmt::format(
        R"({{)"
        R"("type":"clearinghouseState",)"
        R"("user":"{}")"
        R"(}})"sv,
        account_.get_key());
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
    log::warn("DEBUG request={}"sv, request);
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_clearing_house_state_ack(event, sequence);
    };
    (*connection_)("clearing-house-state"sv, request, callback);
  });
}

void OrderEntry::get_clearing_house_state_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = OrderEntryState::CLEARING_HOUSE_STATE;
  profile_.clearing_house_state_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      log::warn("DEBUG {}"sv, body);
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        json::GetClearingHouseStateAck clearing_house_state_ack{body, decode_buffer_};
        Trace event_2{event, clearing_house_state_ack};
        (*this)(event_2);
        download_.check(STATE);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::GetClearingHouseStateAck> const &event) {
  auto &[trace_info, clearing_house_state_ack] = event;
  log::info<4>("clearing_house_state_ack={}"sv, clearing_house_state_ack);
}

// spot-clearing-house-state

void OrderEntry::get_spot_clearing_house_state() {
  profile_.spot_clearing_house_state([&]() {
    auto body = fmt::format(
        R"({{)"
        R"("type":"spotClearinghouseState",)"
        R"("user":"{}")"
        R"(}})"sv,
        account_.get_key());
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
    log::warn("DEBUG request={}"sv, request);
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_spot_clearing_house_state_ack(event, sequence);
    };
    (*connection_)("spot-clearing-house-state"sv, request, callback);
  });
}

void OrderEntry::get_spot_clearing_house_state_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = OrderEntryState::SPOT_CLEARING_HOUSE_STATE;
  profile_.spot_clearing_house_state_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      log::warn("DEBUG {}"sv, body);
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        json::GetSpotClearingHouseStateAck spot_clearing_house_state_ack{body, decode_buffer_};
        Trace event_2{event, spot_clearing_house_state_ack};
        (*this)(event_2);
        download_.check(STATE);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::GetSpotClearingHouseStateAck> const &event) {
  auto &[trace_info, spot_clearing_house_state_ack] = event;
  log::info<4>("spot_clearing_house_state_ack={}"sv, spot_clearing_house_state_ack);
}

// open-orders

void OrderEntry::get_open_orders() {
  profile_.open_orders([&]() {
    auto body = fmt::format(
        R"({{)"
        R"("type":"openOrders",)"
        R"("user":"{}")"
        R"(}})"sv,
        account_.get_key());
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
    log::warn("DEBUG request={}"sv, request);
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_open_orders_ack(event, sequence);
    };
    (*connection_)("open-orders"sv, request, callback);
  });
}

void OrderEntry::get_open_orders_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = OrderEntryState::OPEN_ORDERS;
  profile_.open_orders_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      log::warn("DEBUG {}"sv, body);
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        json::GetOpenOrdersAck open_orders_ack{body, decode_buffer_};
        Trace event_2{event, open_orders_ack};
        (*this)(event_2);
        download_.check(STATE);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::GetOpenOrdersAck> const &event) {
  auto &[trace_info, open_orders_ack] = event;
  log::info<4>("open_orders_ack={}"sv, open_orders_ack);
}

// helpers

void OrderEntry::process_response(web::rest::Response const &response, auto error_handler, auto success_handler) {
  try {
    auto [status, category, body] = response.result();
    switch (category) {
      using enum web::http::Category;
      case UNKNOWN:
      case INFORMATIONAL_RESPONSE:
        response.expect(web::http::Status::OK);  // throws
        break;
      case SUCCESS:
        success_handler(body);
        break;
      case REDIRECTION:
        log::fatal("Unexpected: URL is being redirected"sv);
      case CLIENT_ERROR: {
        auto message = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, message);
        break;
      }
      case SERVER_ERROR: {
        auto message = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, message);
        break;
      }
    }
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
