/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/drop_copy.hpp"

#include <algorithm>
#include <utility>

#include "roq/logging.hpp"

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/charconv/to_string.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/hyperliquid/json/map.hpp"
#include "roq/hyperliquid/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {

// === CONSTANTS ===

namespace {
auto const NAME = "md"sv;

auto const SUPPORTS = Mask{
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::FUNDS,
};

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
      .query = {},
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
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, ping_frequency_{shared.settings.ws.ping_freq},
      connection_{create_connection(*this, shared.settings, context)}, decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .pong = create_metrics(shared.settings, name_, "pong"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .subscription_response = create_metrics(shared.settings, name_, "subscription_response"sv),
          .user = create_metrics(shared.settings, name_, "user"sv),
          .user_fundings = create_metrics(shared.settings, name_, "user_fundings"sv),
          .user_fills = create_metrics(shared.settings, name_, "user_fills"sv),
          .order_updates = create_metrics(shared.settings, name_, "order_updates"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared} {
}

void DropCopy::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopy::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopy::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (ready() && next_ping_ < now) {
    send_ping(now);
  }
}

void DropCopy::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.pong, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.subscription_response, metrics::Type::PROFILE)
      .write(profile_.user, metrics::Type::PROFILE)
      .write(profile_.user_fundings, metrics::Type::PROFILE)
      .write(profile_.user_fills, metrics::Type::PROFILE)
      .write(profile_.order_updates, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void DropCopy::operator()(web::socket::Client::Connected const &) {
}

void DropCopy::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
}

void DropCopy::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::READY);
  subscribe();
}

void DropCopy::operator()(web::socket::Client::Close const &) {
}

void DropCopy::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void DropCopy::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
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

void DropCopy::subscribe() {
  subscribe("notification"sv);
  subscribe("orderUpdates"sv);
  subscribe("userEvents"sv);
  subscribe("userFills"sv);
  subscribe("userFundings"sv);
  // subscribe("userNonFundingLedgerUpdates"sv);
}

void DropCopy::subscribe(std::string_view const &type) {
  auto message = fmt::format(
      R"({{)"
      R"("method":"subscribe",)"
      R"("subscription":{{)"
      R"("type":"{}",)"
      R"("user":"{}")"
      R"(}})"
      R"(}})"sv,
      type,
      account_.get_key());
  log::warn("DEBUG {}"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::send_ping(std::chrono::nanoseconds now) {
  assert(ping_frequency_.count() > 0);
  next_ping_ = now + ping_frequency_ / 2;
  auto message = R"({"method":"ping"})";
  (*connection_).send_text(message);
}

void DropCopy::parse(std::string_view const &message) {
  profile_.parse([&]() {
    log::warn("DEBUG {}"sv, message);
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::Parser::dispatch(*this, message, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

// json::Parser::Handler

void DropCopy::operator()(Trace<json::Pong> const &event) {
  profile_.pong([&]() {
    auto &[trace_info, pong] = event;
    log::info<5>("pong={}"sv, pong);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void DropCopy::operator()(Trace<json::Error> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::error("error={}"sv, error);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void DropCopy::operator()(Trace<json::SubscriptionResponse> const &event) {
  profile_.subscription_response([&]() {
    auto &[trace_info, subscription_response] = event;
    log::info<2>("subscription_response={}"sv, subscription_response);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void DropCopy::operator()(Trace<json::BBO> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::L2Book> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Trades> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::ActiveAssetCtx> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::SpotMeta> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::User> const &event) {
  profile_.user([&]() {
    auto &[trace_info, user] = event;
    log::warn("DEBUG {}"sv, user);
    // XXX FIXME TODO funding
    // XXX FIXME TODO fills
  });
}

void DropCopy::operator()(Trace<json::UserFundings> const &event) {
  profile_.user_fundings([&]() {
    auto &[trace_info, user_fundings] = event;
    log::warn("DEBUG {}"sv, user_fundings);
  });
}

void DropCopy::operator()(Trace<json::UserFills> const &event) {
  profile_.user_fills([&]() {
    auto &[trace_info, user_fills] = event;
    log::warn("DEBUG {}"sv, user_fills);
  });
}

void DropCopy::operator()(Trace<json::OrderUpdates> const &event) {
  profile_.order_updates([&]() {
    auto &[trace_info, order_updates] = event;
    log::info<2>("order_updates={}"sv, order_updates);
    for (auto &item : order_updates.data) {
      log::warn("DEBUG item={}"sv, item);
      auto external_order_id = fmt::format("{}"sv, item.order.oid);
      auto client_order_id = json::get_client_order_id(item.order.cloid);
      auto order_status = map(item.status).get<OrderStatus>();
      auto error = [&]() -> Error {
        if (order_status == OrderStatus::REJECTED) {
          return Error::UNKNOWN;
        }
        return {};
      }();
      auto text = [&]() -> std::string_view {
        if (order_status == OrderStatus::REJECTED) {
          return item.status.as_raw_text();
        }
        return {};
      }();
      auto traded_quantity = item.order.orig_sz - item.order.sz;
      auto order_update = server::oms::OrderUpdate{
          .account = account_.name,
          .exchange = shared_.settings.exchange,
          .symbol = item.order.coin,
          .side = map(item.order.side),
          .position_effect = {},
          .margin_mode = {},
          .max_show_quantity = NaN,
          .order_type = OrderType::LIMIT,     // note!
          .time_in_force = TimeInForce::GTC,  // note! we need this to always be GTC due to modify order using it
          .execution_instructions = {},
          .create_time_utc = item.order.timestamp,
          .update_time_utc = item.status_timestamp,
          .external_account = {},
          .external_order_id = external_order_id,
          .client_order_id = client_order_id,
          .order_status = order_status,
          .error = error,
          .text = text,
          .quantity = item.order.orig_sz,
          .price = item.order.limit_px,
          .stop_price = NaN,
          .leverage = NaN,
          .remaining_quantity = item.order.sz,
          .traded_quantity = traded_quantity,
          .average_traded_price = NaN,
          .last_traded_quantity = {},
          .last_traded_price = {},
          .last_liquidity = {},
          .routing_id = {},
          .max_request_version = {},
          .max_response_version = {},
          .max_accepted_version = {},
          .update_type = UpdateType::INCREMENTAL,
          .sending_time_utc = {},
      };
      log::warn("DEBUG order_update={}"sv, order_update);
      Trace event_2{trace_info, order_update};
      (*this)(event_2, client_order_id);
    }
  });
}

void DropCopy::operator()(Trace<server::oms::OrderUpdate> const &event, std::string_view const &client_order_id) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

}  // namespace hyperliquid
}  // namespace roq
