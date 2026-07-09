/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/gateway/web_socket.hpp"

#include "roq/logging.hpp"

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/charconv/to_string.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/hyperliquid/tools/encoder.hpp"

#include "roq/hyperliquid/protocol/json/map.hpp"
#include "roq/hyperliquid/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::ORDER_ACK,
    SupportType::ORDER,
};

uint64_t const REQUEST_ID = 1'000'000;

size_t const MAX_DECODE_BUFFER_DEPTH = 2;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&settings.ws.uri, 1},
      .host = settings.ws.host,
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = settings.net.disconnect_on_idle_timeout,
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};

auto get_exchange_from_coin(auto &coin, auto &settings) {
  if (settings.aggregator) {
    auto sep = coin.find_first_of(':');
    if (sep == std::string_view::npos) {
      return settings.exchange;
    }
    return coin.substr(0, sep);
  } else {
    return settings.exchange;
  }
}
}  // namespace

// === IMPLEMENTATION ===

WebSocket::WebSocket(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, ping_frequency_{shared.settings.ws.ping_freq},
      connection_{create_connection(*this, shared.settings, context)}, decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      request_id_{stream_id_ * REQUEST_ID},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .pong = create_metrics(shared.settings, name_, "pong"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .subscription_response = create_metrics(shared.settings, name_, "subscription_response"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared} {
}

void WebSocket::operator()(Event<Start> const &) {
  (*connection_).start();
}

void WebSocket::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void WebSocket::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (ready() && next_ping_ < now) {
    send_ping(now);
  }
}

void WebSocket::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.pong, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.subscription_response, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

uint16_t WebSocket::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  auto &[message_info, create_order] = event;
  auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
  auto expires_after_utc = now_utc + shared_.settings.rest.recv_window;
  auto [action, packed] = tools::Encoder::create_order(create_order, order, ref_data, request_id, now_utc, expires_after_utc);
  auto request = account_.sign_l1_action(action, packed, now_utc, expires_after_utc);
  auto message = fmt::format(
      R"({{)"
      R"("method":"post",)"
      R"("id":{},)"
      R"("request":{{)"
      R"("type":"action",)"
      R"("payload":{})"
      R"(}})"
      R"(}})"sv,
      ++request_id_,
      request);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t WebSocket::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  auto &[message_info, modify_order] = event;
  auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
  auto expires_after_utc = now_utc + shared_.settings.rest.recv_window;
  auto [action, packed] = tools::Encoder::modify_order(modify_order, order, ref_data, request_id, previous_request_id, now_utc, expires_after_utc);
  auto request = account_.sign_l1_action(action, packed, now_utc, expires_after_utc);
  auto message = fmt::format(
      R"({{)"
      R"("method":"post",)"
      R"("id":{},)"
      R"("request":{{)"
      R"("type":"action",)"
      R"("payload":{})"
      R"(}})"
      R"(}})"sv,
      ++request_id_,
      request);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t WebSocket::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  auto &[message_info, cancel_order] = event;
  auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
  auto expires_after_utc = now_utc + shared_.settings.rest.recv_window;
  auto [action, packed] = tools::Encoder::cancel_order(cancel_order, order, ref_data, request_id, previous_request_id, now_utc, expires_after_utc);
  auto request = account_.sign_l1_action(action, packed, now_utc, expires_after_utc);
  auto message = fmt::format(
      R"({{)"
      R"("method":"post",)"
      R"("id":{},)"
      R"("request":{{)"
      R"("type":"action",)"
      R"("payload":{})"
      R"(}})"
      R"(}})"sv,
      ++request_id_,
      request);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t WebSocket::operator()(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

// web::socket::Client::Handler

void WebSocket::operator()(web::socket::Client::Connected const &) {
}

void WebSocket::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
}

void WebSocket::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::READY);
}

void WebSocket::operator()(web::socket::Client::Close const &) {
}

void WebSocket::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(shared_.dispatcher, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void WebSocket::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void WebSocket::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

// helpers

void WebSocket::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
  connection_status_ = connection_status;
  TraceInfo trace_info;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = {},
      .supports = SUPPORTS,
      .transport = Transport::TCP,
      .protocol = Protocol::WS,
      .encoding = {Encoding::JSON},
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
      .reason = reason,
      .interface = (*connection_).get_interface(),
      .authority = (*connection_).get_current_authority(),
      .path = (*connection_).get_current_path(),
      .proxy = (*connection_).get_proxy(),
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(shared_.dispatcher, trace_info, stream_status);
}

void WebSocket::send_ping(std::chrono::nanoseconds now) {
  assert(ping_frequency_.count() > 0);
  next_ping_ = now + ping_frequency_ / 2;
  auto message = R"({"method":"ping"})";
  (*connection_).send_text(message);
}

void WebSocket::parse(std::string_view const &message) {
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!protocol::json::Parser::dispatch(*this, message, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

// protocol::json::Parser::Handler

void WebSocket::operator()(Trace<protocol::json::Pong> const &event) {
  profile_.pong([&]() {
    auto &[trace_info, pong] = event;
    log::info<5>("pong={}"sv, pong);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void WebSocket::operator()(Trace<protocol::json::Error> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::error("error={}"sv, error);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void WebSocket::operator()(Trace<protocol::json::SubscriptionResponse> const &event) {
  profile_.subscription_response([&]() {
    auto &[trace_info, subscription_response] = event;
    log::info<2>("subscription_response={}"sv, subscription_response);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void WebSocket::operator()(Trace<protocol::json::BBO> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<protocol::json::L2Book> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<protocol::json::Trades> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<protocol::json::ActiveAssetCtx> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<protocol::json::SpotMeta> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<protocol::json::User> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<protocol::json::UserFundings> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<protocol::json::UserFills> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<protocol::json::OrderUpdates> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<protocol::json::Notification> const &) {
  log::fatal("Unexpected"sv);
}

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
