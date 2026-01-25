#ifndef PRICER_H
#define PRICER_H

#include <asset_class.h>
#include <equity_price_data.h>
#include <future_price_data.h>
#include <get_random.h>
#include <greeks.h>
#include <market_side.h>
#include <option_price_data.h>
#include <option_type.h>
#include <options.h>
#include <order_book.h>
#include <order_type.h>
#include <pricing_data.h>
#include <pricing_utils.h>
#include <time_point.h>

#include <memory>
#include <variant>

namespace solstice::pricing
{

class Pricer
{
   public:
    Pricer(std::shared_ptr<matching::OrderBook> orderBook);

    void update(matching::OrderPtr order);

    template <typename T>
    double updatedDemandFactor(T& priceData)
    {
        double currentDF = priceData.demandFactor();
        double newPrice = priceData.lastPrice();
        double ma = priceData.movingAverage();

        if (priceData.executions() < 2)
        {
            return Random::getRandomDouble(-0.3, 0.3);
        }

        double sigma = priceData.standardDeviation(priceData);
        double noise = Random::getRandomDouble(-0.05, 0.05);
        currentDF += noise;

        double priceDeviation = newPrice - ma;
        // price too high
        if (priceDeviation > 1.5 * sigma)
        {
            currentDF -= 0.15;
        }
        // price too low
        else if (priceDeviation < -1.5 * sigma)
        {
            currentDF += 0.15;
        }

        // Mean reversion toward 0 instead of decay
        // Gently pull toward 0, but don't force it there
        currentDF = currentDF * 0.95 + 0.0 * 0.05;
        currentDF = std::max(-1.0, std::min(1.0, currentDF));

        return currentDF;
    }

    template <typename AssetClass>
    Resolution<PricerDepOrderData> computeOrderData(AssetClass& underlying)
    {
        if constexpr (std::is_same_v<AssetClass, Option>)
        {
            return resolution::err(
                "computeOrderData is not appropriate for options. Use computeOptionData instead.");
        }

        return std::visit(
            [this, &underlying](auto&& underlyingValue) -> Resolution<PricerDepOrderData>
            {
                using T = std::decay_t<decltype(underlyingValue)>;
                if constexpr (std::is_same_v<T, Option>)
                {
                    return resolution::err(
                        "computeOrderData is not appropriate for options. Use computeOptionData "
                        "instead.");
                }
                else
                {
                    auto marketSide = calculateMarketSide(underlyingValue);
                    auto price = calculateMarketPrice(underlyingValue, marketSide);
                    auto qnty = calculateQnty(underlyingValue, marketSide, price);
                    return PricerDepOrderData(underlying, marketSide, price, qnty);
                }
            },
            underlying);
    }

    // options pricing
    double calculateStrikeImpl(PricerDepOptionData& data);

    PricerDepOptionData computeOptionData(Option opt);
    double computeBlackScholes(PricerDepOptionData& data);
    Greeks computeGreeks(OptionOrder& option);

   public:
    // propogate results from market side calc
    double calculateMarketPrice(Equity eq, MarketSide mktSide);
    double calculateMarketPrice(Future fut, MarketSide mktSide);
    double calculateMarketPrice(PricerDepOptionData data, double theoreticalPrice,
                                MarketSide mktSide);

    double calculateMarketPriceImpl(MarketSide mktSide, double lowestAsk, double highestBid,
                                    double demandFactor);

    // propogate results from market side calc and price calc
    int calculateQnty(Equity eq, MarketSide mktSide, double price);
    int calculateQnty(Future fut, MarketSide mktSide, double price);
    int calculateQnty(Option opt, MarketSide mktSide, double price);

   private:
    double generateSeedPrice();

    template <typename Func>
    auto withPriceData(Underlying underlying, Func&& func)
    {
        return std::visit([this, &func](auto asset)
                          { return func(orderBook()->getPriceData(asset)); }, underlying);
    }

    template <typename Func>
    auto withPriceData(matching::OrderPtr order, Func&& func)
    {
        return std::visit([this, &func](auto asset)
                          { return func(orderBook()->getPriceData(asset)); }, order->underlying());
    }

    MarketSide calculateMarketSide(Equity eq);
    MarketSide calculateMarketSide(Future fut);
    MarketSide calculateMarketSide(Option opt);

    MarketSide calculateMarketSideImpl(double probability);

    OrderType getOrderType();

    double calculateCarryAdjustment(Future fut);

    double d_seedPrice;

    std::shared_ptr<matching::OrderBook> orderBook();
    std::shared_ptr<matching::OrderBook> d_orderBook;
};
}  // namespace solstice::pricing

#endif  // PRICER_H
