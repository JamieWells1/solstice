#include <config.h>
#include <types.h>

#include <format>

namespace solstice
{

Config::Config() {}

std::expected<Config, String> Config::instance()
{
    Config config;

    auto isValid = checkConfig(config);
    if (!isValid)
    {
        return std::unexpected(isValid.error());
    }
    return std::move(config);
}

LogLevel Config::logLevel() const { return d_logLevel; }
AssetClass Config::assetClass() const { return d_assetClass; }
int Config::ordersToGenerate() const { return d_ordersToGenerate; }
int Config::underlyingPoolCount() const { return d_underlyingPoolCount; }
int Config::minQnty() const { return d_minQnty; }
int Config::maxQnty() const { return d_maxQnty; }
double Config::minPrice() const { return d_minPrice; }
double Config::maxPrice() const { return d_maxPrice; }
int Config::minExpiryDays() const { return d_minExpiryDays; }
int Config::maxExpiryDays() const { return d_maxExpiryDays; }
bool Config::usePricer() const { return d_usePricer; }
bool Config::enableBroadcaster() const { return d_enableBroadcaster; }
int Config::broadcastInterval() const { return d_broadcastInterval; }

void Config::logLevel(LogLevel level) { d_logLevel = level; }
void Config::assetClass(AssetClass assetClass) { d_assetClass = assetClass; }
void Config::ordersToGenerate(int count) { d_ordersToGenerate = count; }
void Config::underlyingPoolCount(int count) { d_underlyingPoolCount = count; }
void Config::minQnty(int qnty) { d_minQnty = qnty; }
void Config::maxQnty(int qnty) { d_maxQnty = qnty; }
void Config::minPrice(double price) { d_minPrice = price; }
void Config::maxPrice(double price) { d_maxPrice = price; }
void Config::minExpiryDays(int days) { d_minExpiryDays = days; }
void Config::maxExpiryDays(int days) { d_maxExpiryDays = days; }
void Config::usePricer(bool usePricer) { d_usePricer = usePricer; }
void Config::enableBroadcaster(bool enableBroadcaster) { d_enableBroadcaster = enableBroadcaster; }
void Config::broadcastInterval(int broadcastInterval) { d_broadcastInterval = broadcastInterval; }

int Config::initialBalance() const { return d_initialBalance; }

std::expected<void, String> Config::checkConfig(Config& config)
{
    auto values = {double(config.ordersToGenerate()), double(config.minQnty()),
                   double(config.maxQnty()),          double(config.minPrice()),
                   double(config.maxPrice()),         double(config.underlyingPoolCount())};

    if (config.ordersToGenerate() == -1)
    {
        return {};
    }

    for (auto value : values)
    {
        if (value < 0)
        {
            return std::unexpected(std::format("Invalid config value: '{}'\n", value));
        }
    }

    return {};
}

}  // namespace solstice
