/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/hyperliquid/tools/conversions.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

using namespace std::literals;

namespace roq {
namespace hyperliquid {
namespace tools {

// === IMPLEMENTATION ===

double Conversions::roundPrice(double price, int sz_decimals, bool is_spot) {
  // Integer prices > 100k are always allowed
  if (price > 100000.0 && price == std::floor(price)) {
    return price;
  }

  // Round to 5 significant figures (matches Python's :.5g format)
  // Find the order of magnitude
  int magnitude = 0;
  if (price != 0.0) {
    magnitude = static_cast<int>(std::floor(std::log10(std::abs(price))));
  }

  // Round to 5 significant figures
  int sig_figs = 5;
  double scale = std::pow(10.0, magnitude - (sig_figs - 1));
  double sig_figs_price = std::round(price / scale) * scale;

  // Determine max decimals: 6 for perps, 8 for spot
  int max_decimals = is_spot ? 8 : 6;
  int decimals = max_decimals - sz_decimals;

  // Round to the appropriate number of decimal places
  double multiplier = std::pow(10.0, decimals);
  double rounded = std::round(sig_figs_price * multiplier) / multiplier;

  return rounded;
}

double Conversions::roundSize(double size, int sz_decimals) {
  // Round size to szDecimals
  double multiplier = std::pow(10.0, sz_decimals);
  return std::round(size * multiplier) / multiplier;
}

std::string Conversions::floatToWire(double value) {
  // Format to 8 decimal places
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(8) << value;
  std::string rounded = oss.str();

  // Verify no significant rounding occurred
  double back = std::stod(rounded);
  if (std::abs(back - value) >= 1e-12) {
    throw std::runtime_error("floatToWire causes rounding");
  }

  // Handle -0 case
  if (rounded == "-0.00000000") {
    rounded = "0.00000000";
  }

  // Normalize: remove trailing zeros
  size_t decimal_pos = rounded.find('.');
  if (decimal_pos != std::string::npos) {
    // Remove trailing zeros
    size_t last_nonzero = rounded.find_last_not_of('0');
    if (last_nonzero >= decimal_pos) {
      rounded = rounded.substr(0, last_nonzero + 1);
    }
    // Remove decimal point if no fractional part
    if (rounded.back() == '.') {
      rounded.pop_back();
    }
  }

  return rounded;
}

}  // namespace tools
}  // namespace hyperliquid
}  // namespace roq
