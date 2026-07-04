/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/compat.hpp"

#include <memory>
#include <string>
#include <vector>

#include "roq/server.hpp"

#include "roq/utils/container.hpp"

#include "roq/io/context.hpp"

#include "roq/hyperliquid/gateway/account.hpp"
#include "roq/hyperliquid/gateway/config.hpp"
#include "roq/hyperliquid/gateway/settings.hpp"
#include "roq/hyperliquid/gateway/shared.hpp"

#include "roq/hyperliquid/gateway/drop_copy.hpp"
#include "roq/hyperliquid/gateway/market_data.hpp"
#include "roq/hyperliquid/gateway/order_entry.hpp"
#include "roq/hyperliquid/gateway/rest.hpp"
#include "roq/hyperliquid/gateway/web_socket.hpp"

namespace roq {
namespace hyperliquid {
namespace gateway {

struct Controller final : public server::Handler,
                          public Rest::Handler,
                          public MarketData::Handler,
                          public OrderEntry::Handler,
                          public WebSocket::Handler,
                          public DropCopy::Handler {
  ROQ_PUBLIC static std::unique_ptr<server::Handler> create(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  ROQ_PUBLIC static uint8_t parse_api(Settings const &);

  Controller(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  Controller(Controller const &) = delete;

 protected:
  // server::Handler

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Control> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  void operator()(Event<Subscribe> const &) override;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  uint16_t operator()(Event<MassQuote> const &) override;

  uint16_t operator()(Event<CancelQuotes> const &) override;

  void operator()(metrics::Writer &) const override;

  // Rest::Handler

  void operator()(Rest::SymbolsUpdate &) override;

  // helpers

  void ensure_symbol_slices(size_t size);

  template <typename... Args>
  void dispatch(Args &&...);

  template <typename... Args>
  static void dispatch_helper(auto &self, Args &&...);

  OrderEntry &get_order_entry(std::string_view const &account);
  WebSocket &get_web_socket(std::string_view const &account);

 private:
  server::Dispatcher &dispatcher_;
  // accounts
  utils::unordered_map<std::string, std::unique_ptr<Account>> const accounts_;
  // io
  io::Context &context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  Rest rest_;
  std::vector<std::unique_ptr<MarketData>> market_data_;
  utils::unordered_map<std::string, std::unique_ptr<OrderEntry>> order_entry_;
  utils::unordered_map<std::string, std::unique_ptr<WebSocket>> web_socket_;
  utils::unordered_map<std::string, std::unique_ptr<DropCopy>> drop_copy_;
};

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
