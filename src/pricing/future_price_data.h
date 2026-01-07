#ifndef FUTURE_PRICE_DATA_H
#define FUTURE_PRICE_DATA_H

#include <asset_class.h>
#include <pricing_utils.h>

namespace solstice::pricing
{

struct FuturePriceData
{
   public:
    FuturePriceData(Future underlying) : d_future(underlying)
    {
        setInitialDemandFactor(*this);
        setInitialPrice(*this);
        setInitialMovingAverage(*this);
    }

    Future underlying() const;
    int maRange() const;

    double lastPrice() const;
    double highestBid() const;
    double lowestAsk() const;
    double demandFactor() const;
    double movingAverage() const;
    int executions() const;
    double pricesSum() const;
    double pricesSumSquared() const;

    void underlying(Future fut);

    void lastPrice(double newLastPrice);
    void highestBid(double newHighestBid);
    void lowestAsk(double newLowestAsk);
    void demandFactor(double newDemandFactor);
    void movingAverage(double newMovingAverage);
    void incrementExecutions();
    void pricesSum(double newPricesSum);
    void pricesSumSquared(double newPricesSumSquared);

    double standardDeviation(const FuturePriceData& data) const;
    void updateVolatility(double newPrice);
    double volatility() const;

   private:
    static constexpr int d_maRange = 10;
    static constexpr double d_lambda = 0.94;  // EWMA decay factor (~30-day window)

    Future d_future;

    double d_lastPrice = 0.0;
    double d_highestBid = 0.0;
    double d_lowestAsk = 0.0;
    double d_demandFactor = 0.0;
    double d_movingAverage = 0.0;

    // used for pricing calculations
    int d_executions = 0;
    double d_pricesSum = 0.0;
    double d_pricesSumSquared = 0.0;

    // EWMA volatility tracking
    double d_previousPrice = 0.0;
    double d_varianceEWMA = 0.0001;  // small initial variance
};

}  // namespace solstice::pricing

#endif  // FUTURE_PRICE_DATA_H
