#include <equity_price_data.h>

namespace solstice::pricing
{

Equity EquityPriceData::underlying() const { return d_equity; }
double EquityPriceData::lastPrice() const { return d_lastPrice; }
double EquityPriceData::highestBid() const { return d_highestBid; }
double EquityPriceData::lowestAsk() const { return d_lowestAsk; }
double EquityPriceData::demandFactor() const { return d_demandFactor; }
double EquityPriceData::movingAverage() const { return d_movingAverage; }
int EquityPriceData::executions() const { return d_executions; }
int EquityPriceData::maRange() const { return d_maRange; }
double EquityPriceData::pricesSum() const { return d_pricesSum; }
double EquityPriceData::pricesSumSquared() const { return d_pricesSumSquared; }
double EquityPriceData::previousPrice() const { return d_previousPrice; }
double EquityPriceData::varianceEWMA() const { return d_varianceEWMA; }

void EquityPriceData::underlying(Equity newUnderlying) { d_equity = newUnderlying; }
void EquityPriceData::lastPrice(double newLastPrice) { d_lastPrice = newLastPrice; }
void EquityPriceData::highestBid(double newHighestBid) { d_highestBid = newHighestBid; }
void EquityPriceData::lowestAsk(double newLowestAsk) { d_lowestAsk = newLowestAsk; }
void EquityPriceData::demandFactor(double newDemandFactor) { d_demandFactor = newDemandFactor; }
void EquityPriceData::movingAverage(double newMovingAverage) { d_movingAverage = newMovingAverage; }
void EquityPriceData::incrementExecutions() { d_executions++; }
void EquityPriceData::pricesSum(double newPricesSum) { d_pricesSum = newPricesSum; }
void EquityPriceData::pricesSumSquared(double newPricesSumSq)
{
    d_pricesSumSquared = newPricesSumSq;
}
void EquityPriceData::previousPrice(double mostRecentPrice) { d_previousPrice = mostRecentPrice; }
void EquityPriceData::varianceEWMA(double newVariance) { d_varianceEWMA = newVariance; }

double EquityPriceData::standardDeviation(const EquityPriceData& data) const
{
    double n = data.executions();
    if (n < 2) return 0;

    return std::sqrt((data.pricesSumSquared() / n) - std::pow((data.pricesSum() / n), 2));
}

void EquityPriceData::updateVolatility(double newPrice)
{
    // for first trade
    if (d_previousPrice == 0.0)
    {
        d_previousPrice = newPrice;
        return;
    }

    double logReturn = std::log(newPrice / d_previousPrice);

    // Update EWMA variance: variance = λ × variance + (1-λ) × return²
    d_varianceEWMA = d_lambda * d_varianceEWMA + (1.0 - d_lambda) * logReturn * logReturn;
    d_previousPrice = newPrice;
}

double EquityPriceData::volatility() const { return std::sqrt(d_varianceEWMA * 252.0); }

}  // namespace solstice::pricing
