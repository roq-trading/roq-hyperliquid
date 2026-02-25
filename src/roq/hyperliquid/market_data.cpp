/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/market_data.hpp"

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
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
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

MarketData::MarketData(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared, size_t index)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, index_{index}, ping_frequency_{shared.settings.ws.ping_freq},
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
          .bbo = create_metrics(shared.settings, name_, "bbo"sv),
          .l2book = create_metrics(shared.settings, name_, "l2book"sv),
          .trades = create_metrics(shared.settings, name_, "trades"sv),
          .active_asset_ctx = create_metrics(shared.settings, name_, "active_asset_ctx"sv),
          .spot_meta = create_metrics(shared.settings, name_, "spot_meta"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      shared_{shared} {
}

void MarketData::operator()(Event<Start> const &) {
  (*connection_).start();
}

void MarketData::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void MarketData::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (ready() && next_ping_ < now) {
    send_ping(now);
  }
}

void MarketData::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.pong, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.subscription_response, metrics::Type::PROFILE)
      .write(profile_.bbo, metrics::Type::PROFILE)
      .write(profile_.l2book, metrics::Type::PROFILE)
      .write(profile_.trades, metrics::Type::PROFILE)
      .write(profile_.active_asset_ctx, metrics::Type::PROFILE)
      .write(profile_.spot_meta, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void MarketData::subscribe(size_t start_from) {
  if (ready()) {
    subscribe(shared_.symbols.get_slice(index_, start_from));
  }
}

void MarketData::operator()(web::socket::Client::Connected const &) {
}

void MarketData::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
}

void MarketData::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::READY);
  // note! we can request reference data through WS -- however, this is not great because we want to slice symbols
  // get_spot_meta();
  subscribe();
}

void MarketData::operator()(web::socket::Client::Close const &) {
}

void MarketData::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void MarketData::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
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
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

void MarketData::get_spot_meta() {
  auto message = fmt::format(
      R"({{)"
      R"("method":"post",)"
      R"("id":{},)"
      R"("request":{{)"
      R"("type":"info",)"
      R"("payload":{{)"
      R"("type":"spotMeta")"
      R"(}})"
      R"(}})"
      R"(}})"sv,
      ++request_id_);
  (*connection_).send_text(message);
}

void MarketData::subscribe(std::span<Symbol const> const &symbols) {
  if (std::empty(symbols)) {
    return;
  }
  subscribe("bbo", symbols);
  subscribe("l2Book", symbols);
  subscribe("trades", symbols);
  subscribe("activeAssetCtx", symbols);
}

void MarketData::subscribe(std::string_view const &channel, std::span<Symbol const> const &symbols) {
  assert(!std::empty(symbols));
  for (auto &item : symbols) {
    auto message = fmt::format(
        R"({{)"
        R"("method":"subscribe",)"
        R"("subscription":{{)"
        R"("type":"{}",)"
        R"("coin":"{}")"
        R"(}})"
        R"(}})"sv,
        channel,
        item);
    (*connection_).send_text(message);
  }
}

void MarketData::send_ping(std::chrono::nanoseconds now) {
  assert(ping_frequency_.count() > 0);
  next_ping_ = now + ping_frequency_ / 2;
  auto message = R"({"method":"ping"})";
  (*connection_).send_text(message);
}

void MarketData::parse(std::string_view const &message) {
  profile_.parse([&]() {
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

void MarketData::operator()(Trace<json::Pong> const &event) {
  profile_.pong([&]() {
    auto &[trace_info, pong] = event;
    log::info<5>("pong={}"sv, pong);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void MarketData::operator()(Trace<json::Error> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::error("error={}"sv, error);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void MarketData::operator()(Trace<json::SubscriptionResponse> const &event) {
  profile_.subscription_response([&]() {
    auto &[trace_info, subscription_response] = event;
    log::info<2>("subscription_response={}"sv, subscription_response);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void MarketData::operator()(Trace<json::BBO> const &event) {
  profile_.bbo([&]() {
    auto &[trace_info, bbo] = event;
    log::info<2>("bbo={}"sv, bbo);
    (*connection_).touch(trace_info.source_receive_time);
    if (std::size(bbo.data.bbo) == 2) {
      auto exchange = get_exchange_from_coin(bbo.data.coin, shared_.settings);
      auto top_of_book = TopOfBook{
          .stream_id = stream_id_,
          .exchange = exchange,
          .symbol = bbo.data.coin,
          .layer =
              {
                  .bid_price = bbo.data.bbo[0].px,
                  .bid_quantity = bbo.data.bbo[0].sz,
                  .ask_price = bbo.data.bbo[1].px,
                  .ask_quantity = bbo.data.bbo[1].sz,
              },
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = bbo.data.time,
          .exchange_sequence = {},
          .sending_time_utc = {},
      };
      create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
    } else {
      log::warn("bbo={}"sv, bbo);
    }
  });
}

void MarketData::operator()(Trace<json::L2Book> const &event) {
  profile_.bbo([&]() {
    auto &[trace_info, l2book] = event;
    log::info<2>("l2book={}"sv, l2book);
    (*connection_).touch(trace_info.source_receive_time);
    auto helper = [&](auto &result, auto &item) {
      auto mbp_update = MBPUpdate{
          .price = item.px,
          .quantity = item.sz,
          .implied_quantity = NaN,
          .number_of_orders = utils::safe_cast(item.n),
          .update_action = {},
          .price_level = {},
      };
      result.emplace_back(std::move(mbp_update));
    };
    if (std::size(l2book.data.levels) != 2) {
      log::fatal("Unexpected: l2book={}"sv);
    }
    shared_.bids.clear();
    for (auto &item : l2book.data.levels[0].data) {
      helper(shared_.bids, item);
    }
    shared_.asks.clear();
    for (auto &item : l2book.data.levels[1].data) {
      helper(shared_.asks, item);
    }
    if (!(std::empty(shared_.bids) && std::empty(shared_.asks))) {
      auto exchange = get_exchange_from_coin(l2book.data.coin, shared_.settings);
      auto market_by_price_update = MarketByPriceUpdate{
          .stream_id = stream_id_,
          .exchange = exchange,
          .symbol = l2book.data.coin,
          .bids = shared_.bids,
          .asks = shared_.asks,
          .update_type = UpdateType::SNAPSHOT,  // note!
          .exchange_time_utc = l2book.data.time,
          .exchange_sequence = {},
          .sending_time_utc = {},
          .price_precision = {},
          .quantity_precision = {},
          .max_depth = {},
          .checksum = {},
      };
      create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
    }
  });
}

void MarketData::operator()(Trace<json::Trades> const &event) {
  profile_.trades([&]() {
    auto &[trace_info, trades] = event;
    log::info<2>("trades={}"sv, trades);
    (*connection_).touch(trace_info.source_receive_time);
    std::chrono::nanoseconds time = {};
    std::string_view coin;
    shared_.trades.clear();
    auto dispatch = [&]() {
      if (std::empty(shared_.trades)) {
        return;
      }
      auto exchange = get_exchange_from_coin(coin, shared_.settings);
      auto trade_summary = TradeSummary{
          .stream_id = stream_id_,
          .exchange = exchange,
          .symbol = coin,
          .trades = shared_.trades,
          .exchange_time_utc = time,
          .exchange_sequence = {},
          .sending_time_utc = {},
      };
      create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
    };
    for (auto &item : trades.data) {
      if (item.time != time || item.coin != coin) {
        dispatch();
        time = item.time;
        coin = item.coin;
      }
      auto trade = Trade{
          .side = map(item.side),
          .price = item.px,
          .quantity = item.sz,
          .trade_id = {},  // note! see below
          .taker_order_id = {},
          .maker_order_id = {},
      };
      utils::charconv::to_string(std::back_inserter(trade.trade_id), item.tid);
      shared_.trades.emplace_back(std::move(trade));
    }
    dispatch();
  });
}

void MarketData::operator()(Trace<json::ActiveAssetCtx> const &event) {
  profile_.active_asset_ctx([&]() {
    auto &[trace_info, active_asset_ctx] = event;
    log::info<2>("active_asset_ctx={}"sv, active_asset_ctx);
    (*connection_).touch(trace_info.source_receive_time);
    std::array<Statistics, 4> statistics{{
        {
            .type = StatisticsType::OPEN_INTEREST,
            .value = active_asset_ctx.data.ctx.open_interest,
        },
        {
            .type = StatisticsType::TRADE_VOLUME,
            .value = active_asset_ctx.data.ctx.day_ntl_vlm,  // XXX FIXME or day_base_vlm ???
        },
        {
            .type = StatisticsType::SETTLEMENT_PRICE,  // XXX ???
            .value = active_asset_ctx.data.ctx.mark_px,
        },
        {
            .type = StatisticsType::FUNDING_RATE,
            .value = active_asset_ctx.data.ctx.funding,
        },
    }};
    auto exchange = get_exchange_from_coin(active_asset_ctx.data.coin, shared_.settings);
    auto statistics_update = StatisticsUpdate{
        .stream_id = stream_id_,
        .exchange = exchange,
        .symbol = active_asset_ctx.data.coin,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::SpotMeta> const &) {
  profile_.spot_meta([&]() { log::warn("DEBUG"sv); });
}

void MarketData::operator()(Trace<json::User> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::UserFundings> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::UserFills> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::OrderUpdates> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::Notification> const &) {
  log::fatal("Unexpected"sv);
}

}  // namespace hyperliquid
}  // namespace roq
