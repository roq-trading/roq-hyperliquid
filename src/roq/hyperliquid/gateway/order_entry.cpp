/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/gateway/order_entry.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/hyperliquid/protocol/json/cancel_order_ack_parser.hpp"
#include "roq/hyperliquid/protocol/json/create_order_ack_parser.hpp"

#include "roq/hyperliquid/protocol/json/map.hpp"
#include "roq/hyperliquid/protocol/json/utils.hpp"

#include "roq/hyperliquid/tools/encoder.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace gateway {

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

OrderEntry::OrderEntry(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .spot_clearing_house_state = create_metrics(shared.settings, name_, "spot_clearing_house_state"sv),
          .spot_clearing_house_state_ack = create_metrics(shared.settings, name_, "spot_clearing_house_state_ack"sv),
          .clearing_house_state = create_metrics(shared.settings, name_, "clearing_house_state"sv),
          .clearing_house_state_ack = create_metrics(shared.settings, name_, "clearing_house_state_ack"sv),
          .open_orders = create_metrics(shared.settings, name_, "open_orders"sv),
          .open_orders_ack = create_metrics(shared.settings, name_, "open_orders_ack"sv),
          .user_fills = create_metrics(shared.settings, name_, "user_fills"sv),
          .user_fills_ack = create_metrics(shared.settings, name_, "user_fills_ack"sv),
          .create_order = create_metrics(shared.settings, name_, "create_order"sv),
          .create_order_ack = create_metrics(shared.settings, name_, "create_order_ack"sv),
          .modify_order = create_metrics(shared.settings, name_, "modify_order"sv),
          .modify_order_ack = create_metrics(shared.settings, name_, "modify_order_ack"sv),
          .cancel_order = create_metrics(shared.settings, name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(shared.settings, name_, "cancel_order_ack"sv),
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
      .write(profile_.spot_clearing_house_state, metrics::Type::PROFILE)
      .write(profile_.spot_clearing_house_state_ack, metrics::Type::PROFILE)
      .write(profile_.clearing_house_state, metrics::Type::PROFILE)
      .write(profile_.clearing_house_state_ack, metrics::Type::PROFILE)
      .write(profile_.open_orders, metrics::Type::PROFILE)
      .write(profile_.open_orders_ack, metrics::Type::PROFILE)
      .write(profile_.user_fills, metrics::Type::PROFILE)
      .write(profile_.user_fills_ack, metrics::Type::PROFILE)
      .write(profile_.create_order, metrics::Type::PROFILE)
      .write(profile_.create_order_ack, metrics::Type::PROFILE)
      .write(profile_.modify_order, metrics::Type::PROFILE)
      .write(profile_.modify_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_order, metrics::Type::PROFILE)
      .write(profile_.cancel_order_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

uint16_t OrderEntry::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  create_order(event, order, ref_data, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  modify_order(event, order, ref_data, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  cancel_order(event, order, ref_data, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

void OrderEntry::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
  connection_status_ = connection_status;
  TraceInfo trace_info;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = {},
      .supports = SUPPORTS,
      .transport = Transport::TCP,
      .protocol = Protocol::HTTP,
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
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

// web::rest::Client::Handler

void OrderEntry::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
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

uint32_t OrderEntry::download(State state) {
  switch (state) {
    using enum State;
    case UNDEFINED:
      assert(false);
      break;
    case SPOT_CLEARING_HOUSE_STATE:
      (*this)(ConnectionStatus::DOWNLOADING, "spot-clearing-house-state"sv);
      get_spot_clearing_house_state();
      return 1;
    case CLEARING_HOUSE_STATE:
      (*this)(ConnectionStatus::DOWNLOADING, "clearing-house-state"sv);
      get_clearing_house_state(0);
      return 1;
    case OPEN_ORDERS:
      (*this)(ConnectionStatus::DOWNLOADING, "open-orders"sv);
      get_open_orders(0);
      return 1;
    case USER_FILLS:
      if (shared_.settings.download.trades_lookback.count()) {
        (*this)(ConnectionStatus::DOWNLOADING, "user-fills"sv);
        get_user_fills(0);
        return 1;
      } else {
        return 0;
      }
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
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
    // log::warn("DEBUG request={}"sv, request);
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_spot_clearing_house_state_ack(event, sequence);
    };
    (*connection_)("spot-clearing-house-state"sv, request, callback);
  });
}

void OrderEntry::get_spot_clearing_house_state_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = State::SPOT_CLEARING_HOUSE_STATE;
  profile_.spot_clearing_house_state_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      // log::warn("DEBUG {}"sv, body);
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        protocol::json::GetSpotClearingHouseStateAck spot_clearing_house_state_ack{body, decode_buffer_};
        Trace event_2{event, spot_clearing_house_state_ack};
        (*this)(event_2);
        download_.check(STATE);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::GetSpotClearingHouseStateAck> const &event) {
  auto &[trace_info, spot_clearing_house_state_ack] = event;
  log::info<4>("spot_clearing_house_state_ack={}"sv, spot_clearing_house_state_ack);
  // log::warn("DEBUG spot_clearing_house_state_ack={}"sv, spot_clearing_house_state_ack);
}

// clearing-house-state

void OrderEntry::get_clearing_house_state(size_t index) {
  profile_.clearing_house_state([&]() {
    assert(index < std::size(shared_.dex));
    auto &dex = shared_.dex[index];
    auto body = fmt::format(
        R"({{)"
        R"("type":"clearinghouseState",)"
        R"("user":"{}",)"
        R"("dex":"{}")"
        R"(}})"sv,
        account_.get_key(),
        dex.name);
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
    // log::warn("DEBUG request={}"sv, request);
    auto callback = [this, sequence = download_.sequence(), index = index]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_clearing_house_state_ack(event, sequence, index);
    };
    (*connection_)("clearing-house-state"sv, request, callback);
  });
}

void OrderEntry::get_clearing_house_state_ack(Trace<web::rest::Response> const &event, uint32_t sequence, size_t index) {
  auto const STATE = State::CLEARING_HOUSE_STATE;
  profile_.clearing_house_state_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      // log::warn("DEBUG {}"sv, body);
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        protocol::json::GetClearingHouseStateAck clearing_house_state_ack{body, decode_buffer_};
        Trace event_2{event, clearing_house_state_ack};
        (*this)(event_2, index);
        auto next_index = index + 1;
        if (next_index >= std::size(shared_.dex)) {
          download_.check(STATE);
        } else {
          get_clearing_house_state(next_index);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::GetClearingHouseStateAck> const &event, [[maybe_unused]] size_t index) {
  auto &[trace_info, clearing_house_state_ack] = event;
  log::info<4>("clearing_house_state_ack={}"sv, clearing_house_state_ack);
  // log::warn("DEBUG clearing_house_state_ack={}"sv, clearing_house_state_ack);
}

// open-orders

void OrderEntry::get_open_orders(size_t index) {
  profile_.open_orders([&]() {
    assert(index < std::size(shared_.dex));
    auto &dex = shared_.dex[index];
    auto body = fmt::format(
        R"({{)"
        R"("type":"openOrders",)"
        R"("user":"{}",)"
        R"("dex":"{}")"
        R"(}})"sv,
        account_.get_key(),
        dex.name);
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
    // log::warn("DEBUG request={}"sv, request);
    auto callback = [this, sequence = download_.sequence(), index = index]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_open_orders_ack(event, sequence, index);
    };
    (*connection_)("open-orders"sv, request, callback);
  });
}

void OrderEntry::get_open_orders_ack(Trace<web::rest::Response> const &event, uint32_t sequence, size_t index) {
  auto const STATE = State::OPEN_ORDERS;
  profile_.open_orders_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      // log::warn("DEBUG {}"sv, body);
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        protocol::json::GetOpenOrdersAck open_orders_ack{body, decode_buffer_};
        Trace event_2{event, open_orders_ack};
        (*this)(event_2, index);
        auto next_index = index + 1;
        if (next_index >= std::size(shared_.dex)) {
          download_.check(STATE);
        } else {
          get_open_orders(next_index);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::GetOpenOrdersAck> const &event, [[maybe_unused]] size_t index) {
  auto &[trace_info, open_orders_ack] = event;
  log::info<4>("open_orders_ack={}"sv, open_orders_ack);
  for (auto &item : open_orders_ack.data) {
    // log::warn("DEBUG item={}"sv, item);
    auto exchange = get_exchange_from_coin(item.coin, shared_.settings);
    auto external_order_id = fmt::format("{}"sv, item.oid);
    auto client_order_id = protocol::json::get_client_order_id(item.cloid);
    auto traded_quantity = item.orig_sz - item.sz;
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = exchange,
        .symbol = item.coin,
        .side = map(item.side),
        .position_effect = {},
        .margin_mode = {},
        .max_show_quantity = NaN,
        .order_type = OrderType::LIMIT,     // note!
        .time_in_force = TimeInForce::GTC,  // note! we need this to always be GTC due to modify order using it
        .execution_instructions = {},
        .create_time_utc = {},
        .update_time_utc = {},
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = client_order_id,
        .order_status = OrderStatus::WORKING,  // note!
        .error = {},
        .text = {},
        .quantity = item.orig_sz,
        .price = item.limit_px,
        .stop_price = NaN,
        .leverage = NaN,
        .remaining_quantity = item.sz,
        .traded_quantity = traded_quantity,
        .average_traded_price = NaN,
        .last_traded_quantity = {},
        .last_traded_price = {},
        .last_liquidity = {},
        .routing_id = {},
        .max_request_version = {},
        .max_response_version = {},
        .max_accepted_version = {},
        .update_type = UpdateType::SNAPSHOT,
        .sending_time_utc = {},
    };
    // log::warn("DEBUG order_update={}"sv, order_update);
    Trace event_2{trace_info, order_update};
    (*this)(event_2, client_order_id);
  }
}

// user-fills

void OrderEntry::get_user_fills(size_t index) {
  profile_.user_fills([&]() {
    assert(index < std::size(shared_.dex));
    auto &dex = shared_.dex[index];
    auto body = fmt::format(
        R"({{)"
        R"("type":"userFills",)"
        R"("user":"{}",)"
        R"("dex":"{}")"
        R"(}})"sv,
        account_.get_key(),
        dex.name);
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
    // log::warn("DEBUG request={}"sv, request);
    auto callback = [this, sequence = download_.sequence(), index = index]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_user_fills_ack(event, sequence, index);
    };
    (*connection_)("user-fills"sv, request, callback);
  });
}

void OrderEntry::get_user_fills_ack(Trace<web::rest::Response> const &event, uint32_t sequence, size_t index) {
  auto const STATE = State::USER_FILLS;
  profile_.user_fills_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      // log::warn("DEBUG {}"sv, body);
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        protocol::json::GetUserFillsAck user_fills_ack{body, decode_buffer_};
        Trace event_2{event, user_fills_ack};
        (*this)(event_2, index);
        auto next_index = index + 1;
        if (next_index >= std::size(shared_.dex)) {
          download_.check(STATE);
        } else {
          get_user_fills(next_index);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::GetUserFillsAck> const &event, [[maybe_unused]] size_t index) {
  auto &[trace_info, user_fills_ack] = event;
  log::info<4>("user_fills_ack={}"sv, user_fills_ack);
  for (auto &item : user_fills_ack.data) {
    log::info<4>("item={}"sv, item);
  }
}

// create-order

void OrderEntry::create_order(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  profile_.create_order([&]() {
    auto &[message_info, create_order] = event;
    auto send_request = [&](auto &body) {
      auto path = shared_.api.simple.order_create;
      auto request = web::rest::Request{
          .method = web::http::Method::POST,
          .path = path,
          .query = {},
          .accept = web::http::Accept::APPLICATION_JSON,
          .content_type = web::http::ContentType::APPLICATION_JSON,
          .headers = {},
          .body = body,
          .quality_of_service = {},
      };
      auto callback = [this, user_id = message_info.source, order_id = create_order.order_id]([[maybe_unused]] auto &request_id, auto &response) {
        auto version = 1;
        TraceInfo trace_info;
        Trace event{trace_info, response};
        create_order_ack(event, user_id, order_id, version);
      };
      // log::warn(R"(DEBUG request="{}")"sv, request);
      (*connection_)(request_id, request, callback);
    };
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto expires_after_utc = now_utc + shared_.settings.rest.recv_window;
    auto [action, packed] = tools::Encoder::create_order(create_order, order, ref_data, request_id, now_utc, expires_after_utc);
    auto request = account_.sign_l1_action(action, packed, now_utc, expires_after_utc);
    send_request(request);
  });
}

void OrderEntry::create_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.create_order_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::debug(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::CREATE_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .external_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      log::warn(R"(DEBUG body="{}")"sv, body);
      auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, body); };
      auto handle_error_2 = [&](auto &text) { handle_error(Origin::EXCHANGE, RequestStatus::FAILED, Error::UNKNOWN, text); };
      auto handle_success_2 = [&](auto &event_2) {
        auto &statuses = event_2.value.response.data.statuses;
        if (std::empty(statuses)) {
          log::warn(R"(Unexpected: message="{}")"sv, body);
        } else {
          auto &status = statuses[0];  // XXX FIXME TODO process all
          if (std::empty(status.error)) {
            (*this)(event_2, user_id, order_id, version);
          } else {
            handle_error_2(status.error);
          }
        }
      };
      try {
        if (!protocol::json::CreateOrderAckParser::dispatch(
                body, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types, handle_error_2, handle_success_2)) {
          log_message();
        }
      } catch (...) {
        log_message();
        utils::exceptions::Unhandled::terminate();
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(
    Trace<protocol::json::CreateOrderAck> const &, [[maybe_unused]] uint8_t user_id, [[maybe_unused]] uint64_t order_id, [[maybe_unused]] uint32_t version) {
}

// modify-order

void OrderEntry::modify_order(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.modify_order([&]() {
    auto &[message_info, modify_order] = event;
    auto send_request = [&](auto &body) {
      auto path = shared_.api.simple.order_create;
      auto request = web::rest::Request{
          .method = web::http::Method::POST,
          .path = path,
          .query = {},
          .accept = web::http::Accept::APPLICATION_JSON,
          .content_type = web::http::ContentType::APPLICATION_JSON,
          .headers = {},
          .body = body,
          .quality_of_service = {},
      };
      auto callback = [this, user_id = message_info.source, order_id = modify_order.order_id, version = modify_order.version](
                          [[maybe_unused]] auto &request_id, auto &response) {
        TraceInfo trace_info;
        Trace event{trace_info, response};
        modify_order_ack(event, user_id, order_id, version);
      };
      log::warn(R"(DEBUG request="{}")"sv, request);
      (*connection_)(request_id, request, callback);
    };
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto expires_after_utc = now_utc + shared_.settings.rest.recv_window;
    auto [action, packed] = tools::Encoder::modify_order(modify_order, order, ref_data, request_id, previous_request_id, now_utc, expires_after_utc);
    auto request = account_.sign_l1_action(action, packed, now_utc, expires_after_utc);
    send_request(request);
  });
}

void OrderEntry::modify_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.modify_order_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::debug(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::MODIFY_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .external_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      log::warn(R"(DEBUG body="{}")"sv, body);
      protocol::json::ModifyOrderAck cancel_order_ack{body, decode_buffer_};
      Trace event_2{event, cancel_order_ack};
      (*this)(event_2, user_id, order_id, version);
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(
    Trace<protocol::json::ModifyOrderAck> const &, [[maybe_unused]] uint8_t user_id, [[maybe_unused]] uint64_t order_id, [[maybe_unused]] uint32_t version) {
}

// cancel-order

void OrderEntry::cancel_order(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.cancel_order([&]() {
    auto &[message_info, cancel_order] = event;
    auto send_request = [&](auto &body) {
      auto path = shared_.api.simple.order_create;
      auto request = web::rest::Request{
          .method = web::http::Method::POST,
          .path = path,
          .query = {},
          .accept = web::http::Accept::APPLICATION_JSON,
          .content_type = web::http::ContentType::APPLICATION_JSON,
          .headers = {},
          .body = body,
          .quality_of_service = {},
      };
      auto callback = [this, user_id = message_info.source, order_id = cancel_order.order_id, version = cancel_order.version](
                          [[maybe_unused]] auto &request_id, auto &response) {
        TraceInfo trace_info;
        Trace event{trace_info, response};
        cancel_order_ack(event, user_id, order_id, version);
      };
      // log::warn(R"(DEBUG request="{}")"sv, request);
      (*connection_)(request_id, request, callback);
    };
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto expires_after_utc = now_utc + shared_.settings.rest.recv_window;
    auto [action, packed] = tools::Encoder::cancel_order(cancel_order, order, ref_data, request_id, previous_request_id, now_utc, expires_after_utc);
    auto request = account_.sign_l1_action(action, packed, now_utc, expires_after_utc);
    send_request(request);
  });
}

void OrderEntry::cancel_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.cancel_order_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::CANCEL_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .external_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      log::warn(R"(DEBUG body="{}")"sv, body);
      auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, body); };
      auto handle_error_2 = [&](auto &text) { handle_error(Origin::EXCHANGE, RequestStatus::FAILED, Error::UNKNOWN, text); };
      auto handle_success_2 = [&](auto &event_2) {
        auto &statuses = event_2.value.response.data.statuses;
        if (std::empty(statuses)) {
          log::warn(R"(Unexpected: message="{}")"sv, body);
        } else {
          auto &status = statuses[0];  // XXX FIXME TODO process all
          if (std::empty(status.error)) {
            (*this)(event_2, user_id, order_id, version);
          } else {
            handle_error_2(status.error);
          }
        }
      };
      try {
        if (!protocol::json::CancelOrderAckParser::dispatch(
                body, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types, handle_error_2, handle_success_2)) {
          log_message();
        }
      } catch (...) {
        log_message();
        utils::exceptions::Unhandled::terminate();
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(
    Trace<protocol::json::CancelOrderAck> const &, [[maybe_unused]] uint8_t user_id, [[maybe_unused]] uint64_t order_id, [[maybe_unused]] uint32_t version) {
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
      case CLIENT_ERROR:
        switch (status) {
          using enum web::http::Status;
          case TOO_MANY_REQUESTS: {  // 429
            (*connection_).suspend(shared_.settings.misc.suspend_after_429);
            auto message = fmt::format("{}"sv, status);
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::REQUEST_RATE_LIMIT_REACHED, message);
            break;
          }
          default: {
            auto message = fmt::format("{}"sv, status);
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, message);
          }
        }
        break;
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

void OrderEntry::operator()(Trace<server::oms::OrderUpdate> const &event, std::string_view const &client_order_id) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

template <typename... Args>
void OrderEntry::operator()(Trace<server::oms::Response> const &event, uint8_t user_id, uint64_t order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(user_id, order_id, stream_id_, trace_info, response, std::forward<Args>(args)..., []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
  }
}

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
