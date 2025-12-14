#ifndef OPTION_PRICE_DATA_H
#define OPTION_PRICE_DATA_H

#include <asset_class.h>
#include <pricing_utils.h>

namespace solstice::pricing
{

struct OptionPriceData
{
   public:
    OptionPriceData(Option underlying) : d_option(underlying)
    {
        setInitialDemandFactor(*this);
        setInitialPrice(*this);
        setInitialMovingAverage(*this);
    }

    Option underlying();
    int maRange();

    double lastPrice();
    double highestBid();
    double lowestAsk();
    double demandFactor();
    double movingAverage();
    int executions();
    double pricesSum();
    double pricesSumSquared();

    void underlying(Option fut);

    void lastPrice(double newLastPrice);
    void highestBid(double newHighestBid);
    void lowestAsk(double newLowestAsk);
    void demandFactor(double newDemandFactor);
    void movingAverage(double newMovingAverage);
    void incrementExecutions();
    void pricesSum(double newPricesSum);
    void pricesSumSquared(double newPricesSumSquared);

    double standardDeviation(OptionPriceData& data);

   private:
    static constexpr int d_maRange = 10;

    Option d_option;

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

#endif  // OPTION_PRICE_DATA_H
