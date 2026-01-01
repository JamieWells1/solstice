#include <config.h>
#include <get_random.h>
#include <market_side.h>
#include <option_type.h>
#include <pricing_data.h>
#include <types.h>

#include <chrono>
#include <iomanip>
#include <sstream>

#include "greeks.h"

namespace solstice
{

constexpr double PRICE_AS_PCT_OF_UNDERLYING_LOWER_BOUND = 0.01;
constexpr double PRICE_AS_PCT_OF_UNDERLYING_UPPER_BOUND = 0.1;

constexpr double STRIKE_AS_PCT_OF_UNDERLYING_LOWER_BOUND = 0.8;
constexpr double STRIKE_AS_PCT_OF_UNDERLYING_UPPER_BOUND = 1.2;

constexpr double DELTA_LOWER_BOUND = 0.05;
constexpr double DELTA_UPPER_BOUND = 0.95;

constexpr double GAMMA_LOWER_BOUND = 0.001;
constexpr double GAMMA_UPPER_BOUND = 0.15;

constexpr double THETA_LOWER_BOUND = 0.01;
constexpr double THETA_UPPER_BOUND = 0.5;

constexpr double VEGA_LOWER_BOUND = 0.01;
constexpr double VEGA_UPPER_BOUND = 0.8;

std::random_device Random::rd;

String Random::getRandomUid()
{
    static std::mt19937_64 rng(std::random_device{}());
    static std::uniform_int_distribution<uint64_t> dist;
    String uid = std::to_string(dist(rng));

    uid.insert(uid.begin(), 20 - uid.length(), '0');

    return uid;
}

int Random::getRandomInt(int min, int max)
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(min, max);

    return dist(gen);
}

double Random::getRandomDouble(double min, double max)
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dist(min, max);

    double value = dist(gen);

    return std::round(value * 100.0) / 100.0;
}

int Random::getRandomBool()
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, 1);

    return dist(gen) == 1;
}

// ===================================================================
// Spot Values
// ===================================================================

double Random::getRandomSpotPrice(double minPrice, double maxPrice)
{
    return Random::getRandomDouble(minPrice, maxPrice);
}

int Random::getRandomQnty(int minQnty, int maxQnty)
{
    return Random::getRandomInt(minQnty, maxQnty);
}

MarketSide Random::getRandomMarketSide()
{
    if (Random::getRandomBool())
    {
        return MarketSide::Bid;
    }
    else
    {
        return MarketSide::Ask;
    }
}

pricing::PricerDepOrderData Random::generateOrderData(Config& cfg)
{
    double price = Random::getRandomSpotPrice(cfg.minPrice(), cfg.maxPrice());
    int qnty = Random::getRandomQnty(cfg.minQnty(), cfg.maxQnty());
    MarketSide mktSide = Random::getRandomMarketSide();

    return pricing::PricerDepOrderData(mktSide, price, qnty);
}

// ===================================================================
// Options Values
// ===================================================================

double Random::getRandomOptionPrice(Config& cfg)
{
    double pct = getRandomDouble(PRICE_AS_PCT_OF_UNDERLYING_LOWER_BOUND,
                                 PRICE_AS_PCT_OF_UNDERLYING_UPPER_BOUND);
    double price = ((cfg.minPrice() + cfg.maxPrice()) / 2) * pct;
    return std::round(price * 100.0) / 100.0;
}

double Random::getRandomStrike(Config& cfg)
{
    double pct = getRandomDouble(STRIKE_AS_PCT_OF_UNDERLYING_LOWER_BOUND,
                                 STRIKE_AS_PCT_OF_UNDERLYING_UPPER_BOUND);
    double strike = ((cfg.minPrice() + cfg.maxPrice()) / 2) * pct;
    return std::round(strike * 100.0) / 100.0;
}

OptionType Random::getRandomOptionType()
{
    return getRandomBool() ? OptionType::Call : OptionType::Put;
}

String Random::getRandomExpiry(Config& cfg)
{
    int daysFromNow = getRandomInt(cfg.minExpiryDays(), cfg.maxExpiryDays());

    auto now = std::chrono::system_clock::now();
    auto expiry = now + std::chrono::hours(24 * daysFromNow);
    auto expiryTime = std::chrono::system_clock::to_time_t(expiry);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&expiryTime), "%Y-%m-%d");
    return oss.str();
}

double Random::getRandomDelta(OptionType optionType)
{
    double delta = getRandomDouble(DELTA_LOWER_BOUND, DELTA_UPPER_BOUND);
    return optionType == OptionType::Call ? delta : -delta;
}

double Random::getRandomGamma() { return getRandomDouble(GAMMA_LOWER_BOUND, GAMMA_UPPER_BOUND); }

double Random::getRandomTheta() { return -getRandomDouble(THETA_LOWER_BOUND, THETA_UPPER_BOUND); }

double Random::getRandomVega() { return getRandomDouble(VEGA_LOWER_BOUND, VEGA_UPPER_BOUND); }

pricing::PricerDepOptionData generateOptionData(Config& cfg)
{
    MarketSide mktSide = Random::getRandomMarketSide();
    double qnty = Random::getRandomQnty(cfg.minQnty(), cfg.maxQnty());
    double price = Random::getRandomOptionPrice(cfg);
    double strike = Random::getRandomStrike(cfg);
    OptionType optionType = Random::getRandomOptionType();
    String expiry = Random::getRandomExpiry(cfg);

    return pricing::PricerDepOptionData(mktSide, price, qnty, strike, optionType, expiry);
}

pricing::Greeks generateGreeks(pricing::PricerDepOptionData& data)
{
    double d = Random::getRandomDelta(data.optionType());
    double g = Random::getRandomGamma();
    double t = Random::getRandomTheta();
    double v = Random::getRandomVega();

    return pricing::Greeks(d, g, t, v);
}

}  // namespace solstice
