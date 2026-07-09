/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/server/bridge/settings.hpp"

#include "roq/hyperliquid/flags/settings.hpp"

namespace roq {
namespace hyperliquid {
namespace bridge {

struct Settings final : public flags::Settings {
  explicit Settings(args::Parser const &);

  server::bridge::Settings bridge;
};

}  // namespace bridge
}  // namespace hyperliquid
}  // namespace roq

template <>
struct fmt::formatter<roq::hyperliquid::bridge::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::hyperliquid::bridge::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(gateway={}, )"
        R"(bridge={})"
        R"(}})"sv,
        static_cast<roq::hyperliquid::flags::Settings const &>(value),
        value.bridge);
  }
};
