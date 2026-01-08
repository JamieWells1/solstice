#ifndef GETRANDOM_H
#define GETRANDOM_H

#include <config.h>
#include <market_side.h>
#include <option_type.h>
#include <pricing_data.h>
#include <types.h>
#include <greeks.h>

#include <random>

namespace solstice
{

class Random
{
   public:
    static String getRandomUid();

    static int getRandomInt(int min, int max);
    static double getRandomDouble(double min, double max);

    static int getRandomBool();

    // spot values
    static double getRandomSpotPrice(double minPrice, double maxPrice);
    static int getRandomQnty(int minQnty, int maxQnty);
    static MarketSide getRandomMarketSide();
    static std::expected<pricing::PricerDepOrderData, String> generateOrderData(Config& cfg);

    // options values
    static double getRandomOptionPrice(const Config& cfg);
    static double getRandomStrike(const Config& cfg);
    static OptionType getRandomOptionType();
    static double getRandomExpiry(const Config& cfg);
    static double getRandomDelta(OptionType optionType);
    static double getRandomGamma();
    static double getRandomTheta();
    static double getRandomVega();
    static std::expected<pricing::PricerDepOptionData, String> generateOptionData(const Config& cfg);
    static pricing::Greeks generateGreeks(const pricing::PricerDepOptionData& data);

   private:
    static std::random_device rd;
};

}  // namespace solstice

#endif  // GETRANDOM_H
