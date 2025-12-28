#include <future_price_data.h>

namespace solstice::pricing
{

Future FuturePriceData::underlying() const { return d_future; }
double FuturePriceData::lastPrice() const { return d_lastPrice; }
double FuturePriceData::highestBid() const { return d_highestBid; }
double FuturePriceData::lowestAsk() const { return d_lowestAsk; }
double FuturePriceData::demandFactor() const { return d_demandFactor; }
double FuturePriceData::movingAverage() const { return d_movingAverage; }
int FuturePriceData::executions() const { return d_executions; }
int FuturePriceData::maRange() const { return d_maRange; }
double FuturePriceData::pricesSum() const { return d_pricesSum; }
double FuturePriceData::pricesSumSquared() const { return d_pricesSumSquared; }

void FuturePriceData::underlying(Future newUnderlying) { d_future = newUnderlying; }
void FuturePriceData::lastPrice(double newLastPrice) { d_lastPrice = newLastPrice; }
void FuturePriceData::highestBid(double newHighestBid) { d_highestBid = newHighestBid; }
void FuturePriceData::lowestAsk(double newLowestAsk) { d_lowestAsk = newLowestAsk; }
void FuturePriceData::demandFactor(double newDemandFactor) { d_demandFactor = newDemandFactor; }
void FuturePriceData::movingAverage(double newMovingAverage) { d_movingAverage = newMovingAverage; }
void FuturePriceData::incrementExecutions() { d_executions++; }
void FuturePriceData::pricesSum(double newPricesSum) { d_pricesSum = newPricesSum; }
void FuturePriceData::pricesSumSquared(double newPricesSumSq)
{
    d_pricesSumSquared = newPricesSumSq;
}

double FuturePriceData::standardDeviation(const FuturePriceData& data) const
{
    double n = data.executions();
    if (n < 2) return 0;

    return std::sqrt((data.pricesSumSquared() / n) - std::pow((data.pricesSum() / n), 2));
}

}  // namespace solstice::pricing
