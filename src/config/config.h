#ifndef CONFIG_H
#define CONFIG_H

#include <asset_class.h>
#include <log_level.h>
#include <strategy.h>

#include <expected>
#include <string>

namespace solstice
{

struct Config
{
   public:
    // ===================================================================
    // Order Book
    // ===================================================================

    static std::expected<Config, std::string> instance();

    LogLevel logLevel() const;
    AssetClass assetClass() const;
    int ordersToGenerate() const;
    int underlyingPoolCount() const;
    int minQnty() const;
    int maxQnty() const;
    double minPrice() const;
    double maxPrice() const;
    bool usePricer() const;
    bool enableBroadcaster() const;
    int broadcastInterval() const;

    void logLevel(LogLevel level);
    void assetClass(AssetClass assetClass);
    void ordersToGenerate(int count);
    void underlyingPoolCount(int count);
    void minQnty(int qnty);
    void maxQnty(int qnty);
    void minPrice(double price);
    void maxPrice(double price);
    void usePricer(bool usePricer);
    void enableBroadcaster(bool enableBroadcaster);
    void broadcastInterval(int broadcastInterval);

    // ===================================================================
    // Backtesting
    // ===================================================================

    static const strategy::Strategy strategy = strategy::Strategy::SharpMovements;

    int initialBalance();

   private:
    // ===================================================================
    // Order Book
    // ===================================================================
    Config();

    static std::expected<void, std::string> checkConfig(Config& config);

    // set sim log level
    LogLevel d_logLevel = LogLevel::DEBUG;

    // asset class to use in sim
    AssetClass d_assetClass = AssetClass::Equity;

    // number of orders to generate in sim -- set to -1 for infinite orders
    int d_ordersToGenerate = 10000;

    // how many variations of underlying asset class to use in sim (e.g. AAPL, MSFT etc)
    int d_underlyingPoolCount = 10;

    // minimum quantity for randomly generated orders (only applicable if d_usePricer = false)
    int d_minQnty = 1;

    // maximum quantity for randomly generated orders (only applicable if d_usePricer = false)
    int d_maxQnty = 20;

    // minimum price for randomly generated orders (only applicable if d_usePricer = false)
    double d_minPrice = 9.0;

    // maximum price for randomly generated orders (only applicable if d_usePricer = false)
    double d_maxPrice = 10.0;

    // enable use of pricer when generating orders
    bool d_usePricer = true;

    // enable outbound LAN web broadcaster
    bool d_enableBroadcaster = false;

    // broadcast 1 order per x that come in. Higher interval value results in faster broadcasting
    int d_broadcastInterval = 10;

    // ===================================================================
    // Backtesting
    // ===================================================================

    // set initial balance for backtesting strategies
    int d_initialBalance = 10000;
};

}  // namespace solstice

#endif  // CONFIG_H
