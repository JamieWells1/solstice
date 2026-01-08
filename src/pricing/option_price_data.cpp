#include <option_price_data.h>

namespace solstice::pricing
{

Option OptionPriceData::underlying() const { return d_option; }
double OptionPriceData::lastPrice() const { return d_lastPrice; }
double OptionPriceData::highestBid() const { return d_highestBid; }
double OptionPriceData::lowestAsk() const { return d_lowestAsk; }
double OptionPriceData::demandFactor() const { return d_demandFactor; }
double OptionPriceData::movingAverage() const { return d_movingAverage; }
int OptionPriceData::executions() const { return d_executions; }
int OptionPriceData::maRange() const { return d_maRange; }
double OptionPriceData::pricesSum() const { return d_pricesSum; }
double OptionPriceData::pricesSumSquared() const { return d_pricesSumSquared; }

void OptionPriceData::underlying(Option newUnderlying) { d_option = newUnderlying; }
void OptionPriceData::lastPrice(double newLastPrice) { d_lastPrice = newLastPrice; }
void OptionPriceData::highestBid(double newHighestBid) { d_highestBid = newHighestBid; }
void OptionPriceData::lowestAsk(double newLowestAsk) { d_lowestAsk = newLowestAsk; }
void OptionPriceData::demandFactor(double newDemandFactor) { d_demandFactor = newDemandFactor; }
void OptionPriceData::movingAverage(double newMovingAverage) { d_movingAverage = newMovingAverage; }
void OptionPriceData::incrementExecutions() { d_executions++; }
void OptionPriceData::pricesSum(double newPricesSum) { d_pricesSum = newPricesSum; }
void OptionPriceData::pricesSumSquared(double newPricesSumSq)
{
    d_pricesSumSquared = newPricesSumSq;
}

double OptionPriceData::standardDeviation(const OptionPriceData& data) const
{
    double n = data.executions();
    if (n < 2) return 0;

    return std::sqrt((data.pricesSumSquared() / n) - std::pow((data.pricesSum() / n), 2));
}

void OptionPriceData::updateVolatility(double newPrice)
{
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

double OptionPriceData::volatility() const
{
    return std::sqrt(d_varianceEWMA * 252.0);
}

}  // namespace solstice::pricing
