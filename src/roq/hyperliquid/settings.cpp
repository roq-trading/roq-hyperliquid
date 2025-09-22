/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/hyperliquid/settings.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {

Settings::Settings(args::Parser const &args) : Settings{args, flags::Flags::create()} {
}

Settings::Settings(args::Parser const &args, flags::Flags const &flags)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER}, exchange{flags.exchange}, misc{flags::Misc::create()}, rest{flags::REST::create()},
      ws{flags::WS::create()}, mbp{flags::MBP::create()}, request{flags::Request::create()} {
  log::info("settings={}"sv, *this);
}

}  // namespace hyperliquid
}  // namespace roq
