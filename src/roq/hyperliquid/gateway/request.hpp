/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/core/limit/request.hpp"

#include "roq/hyperliquid/gateway/shared.hpp"

namespace roq {
namespace hyperliquid {
namespace gateway {

struct Request final : public core::limit::Request::Handler {
  explicit Request(Shared &);

  // core::Request::Handler

  bool ready() const override;
  bool can_request(TraceInfo const &) override;

  // ...

  core::limit::Request spot_clearing_house_state;
  core::limit::Request clearing_house_state;
  core::limit::Request orders;
  core::limit::Request trades;

 private:
  Shared &shared_;
};

}  // namespace gateway
}  // namespace hyperliquid
}  // namespace roq
