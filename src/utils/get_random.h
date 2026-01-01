#ifndef GETRANDOM_H
#define GETRANDOM_H

#include <market_side.h>
#include <option_type.h>
#include <types.h>

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

    static double getRandomSpotPrice(double minPrice, double maxPrice);
    static int getRandomQnty(int minQnty, int maxQnty);
    static MarketSide getRandomMarketSide();

    // options values
    static double getRandomOptionPrice(double underlyingPrice);
    static double getRandomStrike(double underlyingPrice);
    static OptionType getRandomOptionType();
    static String getRandomExpiry(int minDays, int maxDays);
    static double getRandomDelta(OptionType optionType);
    static double getRandomGamma();
    static double getRandomTheta();
    static double getRandomVega();

   private:
    static std::random_device rd;
};

}  // namespace solstice

#endif  // GETRANDOM_H
