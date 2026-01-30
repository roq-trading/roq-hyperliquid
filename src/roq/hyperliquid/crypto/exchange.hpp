#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "roq/hyperliquid/crypto/signing.hpp"
#include "roq/hyperliquid/crypto/types.hpp"

namespace roq {
namespace hyperliquid {
namespace crypto {

/**
 * Exchange class for trading operations
 */
class Exchange {
 public:
  static constexpr double DEFAULT_SLIPPAGE = 0.05;

  explicit Exchange(
      std::shared_ptr<Wallet> wallet,
      std::string const &base_url = "",
      Meta const *meta = nullptr,
      std::string const &vault_address = "",
      std::string const &account_address = "",
      SpotMeta const *spot_meta = nullptr,
      std::vector<std::string> const *perp_dexs = nullptr,
      int timeout_ms = 30000);

  /**
   * Place a single order
   */
  nlohmann::json order(
      std::string const &coin,
      bool is_buy,
      double sz,
      double limit_px,
      OrderType const &order_type,
      bool reduce_only = false,
      std::optional<Cloid> const &cloid = std::nullopt,
      std::optional<BuilderInfo> const &builder = std::nullopt);

  /**
   * Place multiple orders in a single request
   */
  nlohmann::json bulkOrders(
      std::vector<OrderRequest> const &orders, std::optional<BuilderInfo> const &builder = std::nullopt, std::string const &grouping = "na");

  /**
   * Open a market order
   */
  nlohmann::json marketOpen(
      std::string const &coin,
      bool is_buy,
      double sz,
      std::optional<double> px = std::nullopt,
      double slippage = DEFAULT_SLIPPAGE,
      std::optional<Cloid> const &cloid = std::nullopt,
      std::optional<BuilderInfo> const &builder = std::nullopt);

  /**
   * Close a position with market order
   */
  nlohmann::json marketClose(
      std::string const &coin,
      std::optional<double> sz = std::nullopt,
      std::optional<double> px = std::nullopt,
      double slippage = DEFAULT_SLIPPAGE,
      std::optional<Cloid> const &cloid = std::nullopt,
      std::optional<BuilderInfo> const &builder = std::nullopt);

  /**
   * Cancel an order by OID
   */
  nlohmann::json cancel(std::string const &coin, int64_t oid);

  /**
   * Cancel an order by client order ID
   */
  nlohmann::json cancelByCloid(std::string const &coin, Cloid const &cloid);

  /**
   * Cancel multiple orders
   */
  nlohmann::json bulkCancel(std::vector<CancelRequest> const &cancels);

  /**
   * Cancel multiple orders by CLOID
   */
  nlohmann::json bulkCancelByCloid(std::vector<CancelByCloidRequest> const &cancels);

  /**
   * Modify an existing order
   */
  nlohmann::json modifyOrder(
      OidOrCloid const &oid,
      std::string const &coin,
      bool is_buy,
      double sz,
      double limit_px,
      OrderType const &order_type,
      bool reduce_only = false,
      std::optional<Cloid> const &cloid = std::nullopt);

  /**
   * Modify multiple orders
   */
  nlohmann::json bulkModifyOrders(std::vector<ModifyRequest> const &modifies);

  /**
   * Transfer USD to another address
   */
  nlohmann::json usdTransfer(double amount, std::string const &destination);

  /**
   * Transfer spot tokens to another address
   */
  nlohmann::json spotTransfer(double amount, std::string const &destination, std::string const &token);

  /**
   * Update leverage for a coin
   */
  nlohmann::json updateLeverage(int leverage, std::string const &coin, bool is_cross = true);

  /**
   * Schedule future cancel of all open orders.
   * The time must be at least 5 seconds after the current time.
   * Once the time comes, all open orders will be canceled and a trigger count
   * will be incremented. The max number of triggers per day is 10.
   * This trigger count is reset at 00:00 UTC.
   *
   * @param time If provided, set the cancel time (UTC millis). If nullopt, unsets any scheduled cancel.
   */
  nlohmann::json scheduleCancel(std::optional<int64_t> time = std::nullopt);

  /**
   * Query order status by client order ID.
   * Convenience method that delegates to info_.queryOrderByCloid().
   */
  nlohmann::json queryOrderByCloid(std::string const &user, Cloid const &cloid);

  /**
   * Set expiration time for actions (optional)
   */
  void setExpiresAfter(std::optional<int64_t> expires_after);

  // Public info object for queries
  // Info info_;

  // XXX info

  int nameToAsset(std::string const &name) const;
  std::string const &nameToCoin(std::string const &name) const;
  nlohmann::json userState(std::string const &address, std::string const &dex = "");
  nlohmann::json queryOrderByOid(std::string const &user, int64_t oid);
  int asset_to_sz_decimals(int asset);
  int coin_to_asset(std::string const &coin);

 private:
  nlohmann::json postAction(nlohmann::json const &action, Signature const &signature, int64_t nonce);

  double slippagePrice(std::string const &name, bool is_buy, double slippage, std::optional<double> px = std::nullopt);

  std::shared_ptr<Wallet> wallet_;
  std::string base_url_;
  std::string vault_address_;
  std::string account_address_;
  std::optional<int64_t> expires_after_;
};

}  // namespace crypto
}  // namespace hyperliquid
}  // namespace roq
