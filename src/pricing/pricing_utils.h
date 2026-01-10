#ifndef PRICING_UTILS_H
#define PRICING_UTILS_H

#include <get_random.h>

namespace solstice::pricing
{

double N(double x);

double getBandIncrement(double spotPrice);

template <typename PriceData>
void setInitialDemandFactor(PriceData& underlying)  // type
{
    underlying.demandFactor(Random::getRandomDouble(-1, 1));
}

template <typename PriceData>
void setInitialPrice(PriceData& underlying)
{
    underlying.lastPrice(Random::getRandomDouble(10, 200));
}

template <typename PriceData>
void setInitialMovingAverage(PriceData& underlying)
{
    underlying.movingAverage(underlying.lastPrice());
}

}  // namespace solstice::pricing

#endif  // PRICING_UTILS_H
