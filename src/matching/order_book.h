#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <asset_class.h>
#include <equity_price_data.h>
#include <future_price_data.h>
#include <market_side.h>
#include <option_price_data.h>
#include <order.h>
#include <transaction.h>
#include <types.h>

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>

namespace solstice::pricing
{
class EquityPriceData;
class FuturePriceData;
class OptionPriceData;
}  // namespace solstice::pricing

namespace solstice::matching
{

using OrderPtr = std::shared_ptr<Order>;
using PriceLevelMap = std::map<double, std::deque<OrderPtr>>;

using BidPricesAtPriceLevel = std::set<double, std::greater<double>>;
using askPricesAtPriceLevel = std::set<double, std::less<double>>;

struct ActiveOrders
{
    PriceLevelMap bids;
    PriceLevelMap asks;

    BidPricesAtPriceLevel bidPrices;
    askPricesAtPriceLevel askPrices;
};

class OrderBook
{
    friend class Orchestrator;

   public:
    pricing::EquityPriceData& getPriceData(Equity eq);
    pricing::FuturePriceData& getPriceData(Future fut);
    pricing::OptionPriceData& getPriceData(Option opt);

    void addEquitiesToDataMap();
    void addFuturesToDataMap();
    void addOptionsToDataMap();

    const std::vector<Transaction>& transactions() const;
    const std::expected<double, String> getBestPrice(OrderPtr orderToMatch);

    std::optional<std::reference_wrapper<std::deque<OrderPtr>>> getOrdersDequeAtPrice(
        const OrderPtr order);
    std::deque<OrderPtr>& getOrdersDequeAtPrice(OrderPtr order, int priceToMatch);
    std::deque<OrderPtr>& ordersDequeAtPrice(OrderPtr order);

    std::map<double, std::deque<OrderPtr>>& sameMarketSidePriceLevelMap(OrderPtr order);
    std::map<double, std::deque<OrderPtr>>& oppositeMarketSidePriceLevelMap(OrderPtr order);

    std::expected<std::reference_wrapper<std::deque<OrderPtr>>, String> getPriceLevelOppositeOrders(
        OrderPtr order, double priceToUse);

    void addOrderToBook(OrderPtr order);
    void removeOrderFromBook(OrderPtr orderToRemove);
    void markOrderAsFulfilled(OrderPtr completedOrder, double matchedPrice);

    std::optional<std::reference_wrapper<const ActiveOrders>> getActiveOrders(
        const Underlying& underlying) const;

    template <typename T>
    void initialiseBookAtUnderlyings()
    {
        for (const auto& underlying : underlyingsPool<T>())
        {
            d_activeOrders[underlying];
        }
    }

   private:
    std::expected<std::reference_wrapper<BidPricesAtPriceLevel>, String> getBidPricesAtPriceLevel(
        OrderPtr order);
    std::expected<std::reference_wrapper<askPricesAtPriceLevel>, String> getaskPricesAtPriceLevel(
        OrderPtr order);

    BidPricesAtPriceLevel& setBidPricesAtPriceLevel(OrderPtr order);
    askPricesAtPriceLevel& setAskPricesAtPriceLevel(OrderPtr order);

    std::unordered_map<Underlying, ActiveOrders> d_activeOrders;
    std::vector<Transaction> d_transactions;

    std::unordered_map<Equity, pricing::EquityPriceData> d_equityDataMap;
    std::unordered_map<Future, pricing::FuturePriceData> d_futureDataMap;
    std::unordered_map<Option, pricing::OptionPriceData> d_optionDataMap;
};
}  // namespace solstice::matching

#endif  // ORDERBOOK_H
