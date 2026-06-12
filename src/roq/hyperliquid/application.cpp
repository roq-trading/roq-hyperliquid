/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/application.hpp"

#include "roq/hyperliquid/flags/settings.hpp"

#include "roq/hyperliquid/gateway/config.hpp"
#include "roq/hyperliquid/gateway/controller.hpp"

using namespace std::literals;

namespace roq {
namespace hyperliquid {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  flags::Settings settings{args};
  gateway::Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<gateway::Controller>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace hyperliquid
}  // namespace roq
