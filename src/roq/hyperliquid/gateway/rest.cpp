/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/gateway/rest.hpp"

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/hyperliquid/protocol/json/map.hpp"
#include "roq/hyperliquid/protocol/json/utils.hpp"

#include "roq/hyperliquid/tools/tick_size_steps.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const NAME = "rest"sv;

auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 2;

// uint32_t const OFFSET_SPOT = 10000;
// uint32_t const OFFSET_SWAP = 0;
uint32_t const OFFSET_DEX = 110000;
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

Rest::Rest(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .spot_meta = create_metrics(shared.settings, name_, "spot_meta"sv),
          .spot_meta_ack = create_metrics(shared.settings, name_, "spot_meta_ack"sv),
          .perp_dexs = create_metrics(shared.settings, name_, "perp_dexs"sv),
          .perp_dexs_ack = create_metrics(shared.settings, name_, "perp_dexs_ack"sv),
          .meta = create_metrics(shared.settings, name_, "meta"sv),
          .meta_ack = create_metrics(shared.settings, name_, "meta_ack"sv),
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
      .write(profile_.spot_meta, metrics::Type::PROFILE)
      .write(profile_.spot_meta_ack, metrics::Type::PROFILE)
      .write(profile_.perp_dexs, metrics::Type::PROFILE)
      .write(profile_.perp_dexs_ack, metrics::Type::PROFILE)
      .write(profile_.meta, metrics::Type::PROFILE)
      .write(profile_.meta_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

void Rest::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
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
  create_trace_and_dispatch(shared_.dispatcher, trace_info, stream_status);
}

// web::rest::Client::Handler

void Rest::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
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
  create_trace_and_dispatch(shared_.dispatcher, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

uint32_t Rest::download(State state) {
  switch (state) {
    using enum State;
    case UNDEFINED:
      assert(false);
      break;
    case SPOT_META:
      (*this)(ConnectionStatus::DOWNLOADING, "spot-meta"sv);
      get_spot_meta();
      return 1;
    case PERP_DEXS:
      (*this)(ConnectionStatus::DOWNLOADING, "perp-dexs"sv);
      get_perp_dexs();
      return 1;
    case META:
      (*this)(ConnectionStatus::DOWNLOADING, "meta"sv);
      get_meta(0);
      return 1;
      // return std::size(shared_.dex);
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

// spot-meta

void Rest::get_spot_meta() {
  profile_.spot_meta([&]() {
    auto body = R"({"type":"spotMeta"})"sv;
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
      TraceInfo trade_info;
      Trace event{trade_info, response};
      get_spot_meta_ack(event, sequence);
    };
    (*connection_)("spot-meta"sv, request, callback);
  });
}

void Rest::get_spot_meta_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = State::SPOT_META;
  profile_.spot_meta_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      // log::warn("DEBUG spot_meta={}"sv, body);
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        protocol::json::GetSpotMetaAck spot_meta_ack{body, decode_buffer_};
        Trace event_2{event, spot_meta_ack};
        (*this)(event_2);
        download_.check(STATE);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<protocol::json::GetSpotMetaAck> const &event) {
  auto &[trace_info, spot_meta_ack] = event;
  log::info<4>("spot_meta_ack={}"sv, spot_meta_ack);
  for (size_t i = 0; i < std::size(spot_meta_ack.tokens); ++i) {
    auto &item = spot_meta_ack.tokens[i];
    log::debug("item={}"sv, item);
    // XXX FIXME TODO why did we have this check ???
    // if (item.index != i) {
    //   log::fatal("Unexpected"sv);
    // }
  }
  /*
  for (auto &item : spot_meta_ack.universe) {
    auto discard = shared_.discard_symbol(item.name);
    if (std::size(item.tokens) != 2) {
      log::fatal("Unexpected: item={}"sv, item);
    }
    auto &base = spot_meta_ack.tokens[item.tokens[0]];
    auto &quote = spot_meta_ack.tokens[item.tokens[1]];
    auto tick_size = std::pow(10.0, -static_cast<double>(base.sz_decimals));
    auto trade_vol_step_size = std::pow(10.0, -static_cast<double>(base.sz_decimals));
    auto reference_data = ReferenceData{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = item.name,
        .description = {},
        .security_type = SecurityType::SPOT,
        .external_security_id = utils::safe_cast(item.index + OFFSET_SPOT),
        .cfi_code = {},
        .base_currency = base.name,
        .quote_currency = quote.name,
        .settlement_currency = {},
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = tick_size,
        .tick_size_steps = {},
        .multiplier = NaN,
        .min_notional = NaN,
        .min_trade_vol = NaN,
        .max_trade_vol = NaN,
        .trade_vol_step_size = trade_vol_step_size,
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = {},
        .time_zone = {},
        .issue_date = {},
        .settlement_date = {},
        .expiry_datetime = {},
        .expiry_datetime_utc = {},
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = {},
        .discard = discard,
    };
    create_trace_and_dispatch(shared_.dispatcher, trace_info, reference_data, true);
    if (discard) {
      log::info<1>(R"(Drop symbol="{}")"sv, item.name);
      continue;
    }
  }
  */
}

// perp-dexs

void Rest::get_perp_dexs() {
  profile_.perp_dexs([&]() {
    auto body = R"({"type":"perpDexs"})"sv;
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
      get_perp_dexs_ack(event, sequence);
    };
    (*connection_)("perp-dexs"sv, request, callback);
  });
}

void Rest::get_perp_dexs_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = State::PERP_DEXS;
  profile_.perp_dexs_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      // log::warn("DEBUG perp_dexs={}"sv, body);
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        protocol::json::GetPerpDexsAck perp_dexs_ack{body, decode_buffer_};
        Trace event_2{event, perp_dexs_ack};
        (*this)(event_2);
        download_.check(STATE);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<protocol::json::GetPerpDexsAck> const &event) {
  auto &[trace_info, perp_dexs_ack] = event;
  log::info<4>("perp_dexs_ack={}"sv, perp_dexs_ack);
  shared_.dex.reserve(std::size(perp_dexs_ack.data));
  for (size_t i = 0; i < std::size(perp_dexs_ack.data); ++i) {
    auto &item = perp_dexs_ack.data[i];
    auto asset_id_offset = static_cast<int32_t>(i * OFFSET_DEX);
    // log::warn("DEBUG item={}"sv, item);
    for (auto &item_2 : shared_.dex) {
      if (item_2.name == item.name) {
        if (item_2.asset_id_offset != asset_id_offset) {
          log::fatal("Unexpected: {}"sv, item);
        }
        continue;
      }
    }
    auto discard = !std::empty(item.name) && std::ranges::find(shared_.settings.dex, item.name) == std::end(shared_.settings.dex);
    if (discard) {
      continue;
    }
    auto dex = Shared::Dex{
        .name = std::string{item.name},
        .asset_id_offset = asset_id_offset,
    };
    shared_.dex.emplace_back(std::move(dex));
  }
  for (size_t i = 0; i < std::size(shared_.dex); ++i) {
    auto &item = shared_.dex[i];
    log::warn(R"(DEBUG [{}] name="{}", asset_id_offset={})"sv, i, item.name, item.asset_id_offset);
  }
}

// meta

void Rest::get_meta(size_t index) {
  profile_.meta([&]() {
    assert(index < std::size(shared_.dex));
    auto &dex = shared_.dex[index];
    auto body = fmt::format(
        R"({{)"
        R"("type":"meta",)"
        R"("dex":"{}")"
        R"(}})"sv,
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
    auto callback = [this, sequence = download_.sequence(), index = index]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_meta_ack(event, sequence, index);
    };
    (*connection_)("meta"sv, request, callback);
  });
}

void Rest::get_meta_ack(Trace<web::rest::Response> const &event, uint32_t sequence, size_t index) {
  auto const STATE = State::META;
  profile_.meta_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      // log::warn("DEBUG meta={}"sv, body);
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        protocol::json::GetMetaAck meta_ack{body, decode_buffer_};
        Trace event_2{event, meta_ack};
        (*this)(event_2, index);
        auto next_index = index + 1;
        if (next_index >= std::size(shared_.dex)) {
          download_.check(STATE);
        } else {
          get_meta(next_index);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

// XXX TODO symbols update => trigger market data connection
void Rest::operator()(Trace<protocol::json::GetMetaAck> const &event, size_t index) {
  auto &[trace_info, meta_ack] = event;
  log::info<4>("meta_ack={}"sv, meta_ack);
  assert(index < std::size(shared_.dex));
  auto &dex = shared_.dex[index];
  std::vector<Symbol> symbols;
  symbols.reserve(std::size(meta_ack.universe));  // alloc
  size_t counter = 0;
  for (size_t i = 0; i < std::size(meta_ack.universe); ++i) {
    auto &item = meta_ack.universe[i];
    auto discard = shared_.dispatcher.discard_symbol(item.name);
    auto exchange = get_exchange_from_coin(item.name, shared_.settings);
    auto tick_size = std::pow(10.0, -static_cast<double>(item.sz_decimals));
    auto tick_size_steps = tools::TickSizeSteps::get(item.sz_decimals, false);
    auto trade_vol_step_size = std::pow(10.0, -static_cast<double>(item.sz_decimals));
    auto reference_data = ReferenceData{
        .stream_id = stream_id_,
        .exchange = exchange,
        .symbol = item.name,
        .description = {},
        .security_type = SecurityType::SWAP,
        .external_security_id = utils::safe_cast(dex.asset_id_offset + i),
        .cfi_code = {},
        .base_currency = {},
        .quote_currency = {},
        .settlement_currency = {},
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = tick_size,
        .tick_size_steps = tick_size_steps,
        .multiplier = NaN,
        .min_notional = NaN,
        .min_trade_vol = NaN,
        .max_trade_vol = NaN,
        .trade_vol_step_size = trade_vol_step_size,
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = {},
        .time_zone = {},
        .issue_date = {},
        .settlement_date = {},
        .expiry_datetime = {},
        .expiry_datetime_utc = {},
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = {},
        .discard = discard,
    };
    create_trace_and_dispatch(shared_.dispatcher, trace_info, reference_data, true);
    if (discard) {
      log::info<1>(R"(Drop symbol="{}")"sv, item.name);
      continue;
    }
    if (shared_.all_symbols.emplace(item.name).second) {  // only include new
      symbols.emplace_back(item.name);
    }
    ++counter;
  }
  auto symbols_update = SymbolsUpdate{
      .symbols = symbols,
  };
  handler_(symbols_update);
  if (counter > 0) {
    log::info("Symbols {} / {}"sv, counter, std::size(meta_ack.universe));
  }
}

// helpers

void Rest::process_response(web::rest::Response const &response, auto error_handler, auto success_handler) {
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

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
