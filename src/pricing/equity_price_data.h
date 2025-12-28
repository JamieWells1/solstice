#ifndef EQUITY_PRICE_DATA_H
#define EQUITY_PRICE_DATA_H

#include <asset_class.h>
#include <pricing_utils.h>

namespace solstice::pricing
{

struct EquityPriceData
{
   public:
    EquityPriceData(Equity underlying) : d_equity(underlying)
    {
        setInitialDemandFactor(*this);
        setInitialPrice(*this);
        setInitialMovingAverage(*this);
    }

    Equity underlying() const;
    int maRange() const;

    double lastPrice() const;
    double highestBid() const;
    double lowestAsk() const;
    double demandFactor() const;
    double movingAverage() const;
    int executions() const;
    double pricesSum() const;
    double pricesSumSquared() const;

    void underlying(Equity eq);

    void lastPrice(double newLastPrice);
    void highestBid(double newHighestBid);
    void lowestAsk(double newLowestAsk);
    void demandFactor(double newDemandFactor);
    void movingAverage(double newMovingAverage);
    void incrementExecutions();
    void pricesSum(double newPricesSum);
    void pricesSumSquared(double newPricesSumSquared);

    double standardDeviation(const EquityPriceData& data) const;

   private:
    static constexpr int d_maRange = 10;

    Equity d_equity;

    double d_lastPrice = 0.0;
    double d_highestBid = 0.0;
    double d_lowestAsk = 0.0;
    double d_demandFactor = 0.0;
    double d_movingAverage = 0.0;

    // used for pricing calculations
    int d_executions = 0;
    double d_pricesSum = 0.0;
    double d_pricesSumSquared = 0.0;
};

}  // namespace solstice::pricing

#endif  // EQUITY_PRICE_DATA_H
