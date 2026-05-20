/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/application.hpp"

#include "roq/hyperliquid/flags/settings.hpp"

#include "roq/hyperliquid/gateway/config.hpp"
#include "roq/hyperliquid/gateway/controller.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {

// === CONSTANTS ===

namespace {
uint8_t const API_2 = {};
}

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  flags::Settings settings{args};
  gateway::Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading2<gateway::Controller>{settings, config, *context, API_2}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace hyperliquid
}  // namespace roq
