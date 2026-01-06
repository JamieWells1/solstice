#include <asset_class.h>
#include <equity_price_data.h>
#include <future_price_data.h>
#include <option_price_data.h>
#include <market_side.h>
#include <matcher.h>
#include <order.h>
#include <order_book.h>
#include <transaction.h>
#include <truncate.h>
#include <types.h>

#include <cstddef>
#include <deque>
#include <memory>

namespace solstice::matching
{

pricing::EquityPriceData& OrderBook::getPriceData(Equity eq)
{
    return d_equityDataMap.at(eq);
}

pricing::FuturePriceData& OrderBook::getPriceData(Future fut)
{
    return d_futureDataMap.at(fut);
}

pricing::OptionPriceData& OrderBook::getPriceData(Option opt)
{
    return d_optionDataMap.at(opt);
}

void OrderBook::addEquitiesToDataMap()
{
    for (const auto& underlying : underlyingsPool<Equity>())
    {
        d_equityDataMap.emplace(underlying, pricing::EquityPriceData(underlying));
    }
}

void OrderBook::addFuturesToDataMap()
{
    for (const auto& underlying : underlyingsPool<Future>())
    {
        d_futureDataMap.emplace(underlying, pricing::FuturePriceData(underlying));
    }
}

void OrderBook::addOptionsToDataMap()
{
    for (const auto& underlying : underlyingsPool<Option>())
    {
        d_optionDataMap.emplace(underlying, pricing::OptionPriceData(underlying));
    }
}

const std::vector<Transaction>& OrderBook::transactions() const { return d_transactions; }

std::optional<std::reference_wrapper<std::deque<OrderPtr>>> OrderBook::getOrdersDequeAtPrice(
    OrderPtr order)
{
    auto it = d_activeOrders.find(order->underlying());
    if (it == d_activeOrders.end())
    {
        return std::nullopt;
    }

    ActiveOrders& book = d_activeOrders.at(order->underlying());

    return (order->marketSide() == MarketSide::Bid) ? book.bids.at(order->price())
                                                    : book.asks.at(order->price());
}

std::deque<OrderPtr>& OrderBook::getOrdersDequeAtPrice(OrderPtr order, int priceToMatch)
{
    auto& book = d_activeOrders.at(order->underlying());

    return (order->marketSide() == MarketSide::Bid) ? book.bids.at(priceToMatch)
                                                    : book.asks.at(priceToMatch);
}

std::deque<OrderPtr>& OrderBook::ordersDequeAtPrice(OrderPtr order)
{
    auto& book = d_activeOrders[order->underlying()];

    return (order->marketSide() == MarketSide::Bid) ? book.bids[order->price()]
                                                    : book.asks[order->price()];
}

std::expected<std::reference_wrapper<std::deque<OrderPtr>>, String>
OrderBook::getPriceLevelOppositeOrders(OrderPtr order, double priceToUse)
{
    auto it = d_activeOrders.find(order->underlying());
    if (it == d_activeOrders.end())
    {
        return std::unexpected(std::format("No orders at ticker {} on opposite order side\n",
                                           to_string(order->underlying())));
    }

    ActiveOrders& book = d_activeOrders.at(order->underlying());

    if (order->marketSide() == MarketSide::Bid)
    {
        auto priceIt = book.asks.find(priceToUse);
        if (priceIt == book.asks.end() || priceIt->second.empty())
        {
            return std::unexpected(std::format("No prices at ticker {} on opposite order side\n",
                                               to_string(order->underlying())));
        }
        return std::ref(priceIt->second);
    }
    else
    {
        auto priceIt = book.bids.find(priceToUse);
        if (priceIt == book.bids.end() || priceIt->second.empty())
        {
            return std::unexpected(std::format("No prices at ticker {} on opposite order side\n",
                                               to_string(order->underlying())));
        }
        return std::ref(priceIt->second);
    }
}

std::map<double, std::deque<OrderPtr>>& OrderBook::sameMarketSidePriceLevelMap(OrderPtr order)
{
    auto& book = d_activeOrders.at(order->underlying());

    return (order->marketSide() == MarketSide::Bid) ? book.bids : book.asks;
}

std::map<double, std::deque<OrderPtr>>& OrderBook::oppositeMarketSidePriceLevelMap(OrderPtr order)
{
    auto& book = d_activeOrders.at(order->underlying());

    return (order->marketSide() == MarketSide::Bid) ? book.asks : book.bids;
}

std::expected<std::reference_wrapper<BidPricesAtPriceLevel>, String>
OrderBook::getBidPricesAtPriceLevel(OrderPtr order)
{
    auto it = d_activeOrders.find(order->underlying());
    if (it == d_activeOrders.end())
    {
        return std::unexpected(
            std::format("No bid prices available for ticker {}\n", to_string(order->underlying())));
    }

    return std::ref(it->second.bidPrices);
}

std::set<double, std::greater<double>>& OrderBook::setBidPricesAtPriceLevel(OrderPtr order)
{
    auto& bidsSet = d_activeOrders[order->underlying()].bidPrices;

    return bidsSet;
}

std::expected<std::reference_wrapper<askPricesAtPriceLevel>, String>
OrderBook::getaskPricesAtPriceLevel(OrderPtr order)
{
    auto it = d_activeOrders.find(order->underlying());
    if (it == d_activeOrders.end())
    {
        return std::unexpected(
            std::format("No ask prices available for ticker {}\n", to_string(order->underlying())));
    }

    return std::ref(it->second.askPrices);
}

std::set<double, std::less<double>>& OrderBook::setAskPricesAtPriceLevel(OrderPtr order)
{
    auto& asksSet = d_activeOrders[order->underlying()].askPrices;

    return asksSet;
}

const std::expected<double, String> OrderBook::getBestPrice(OrderPtr orderToMatch)
{
    if (orderToMatch->marketSide() == MarketSide::Bid)
    {
        auto askPricesSet = getaskPricesAtPriceLevel(orderToMatch);
        if (!askPricesSet)
        {
            return std::unexpected(askPricesSet.error());
        }

        auto& askPrices = askPricesSet->get();

        if (askPrices.size() == 0)
        {
            return std::unexpected(std::format("No ask orders found for ticker {}\n",
                                               to_string(orderToMatch->underlying())));
        }

        double lowestaskPrice = *askPrices.begin();
        if (lowestaskPrice > orderToMatch->price())
        {
            return std::unexpected(
                "No matching ask orders lower than or equal to bid "
                "price\n");
        }

        return lowestaskPrice;
    }
    else
    {
        auto bidPricesSet = getBidPricesAtPriceLevel(orderToMatch);
        if (!bidPricesSet)
        {
            return std::unexpected(bidPricesSet.error());
        }

        auto& bidPrices = bidPricesSet->get();

        if (bidPrices.size() == 0)
        {
            return std::unexpected(std::format("No bid orders found for ticker {}\n",
                                               to_string(orderToMatch->underlying())));
        }

        // find highest bid price at or below target price
        double highestBidPrice = *bidPrices.begin();
        if (highestBidPrice < orderToMatch->price())
        {
            return std::unexpected(
                "No matching bid orders lower than or equal to bid "
                "price\n");
        }

        return highestBidPrice;
    }
}

void OrderBook::addOrderToBook(OrderPtr order)
{
    // add price to price lookup map
    if (order->marketSide() == MarketSide::Bid)
    {
        setBidPricesAtPriceLevel(order).insert(order->price());
    }
    else
    {
        setAskPricesAtPriceLevel(order).insert(order->price());
    }

    // add order to map of active orders safely
    ordersDequeAtPrice(order).push_back(order);
}

void OrderBook::removeOrderFromBook(OrderPtr orderToRemove)
{
    auto& priceDeque = ordersDequeAtPrice(orderToRemove);
    for (size_t i = 0; i < priceDeque.size(); i++)
    {
        if (orderToRemove->uid() == priceDeque[i]->uid())
        {
            priceDeque.erase(priceDeque.begin() + i);
            break;
        }
    }
}

std::optional<std::reference_wrapper<const ActiveOrders>> OrderBook::getActiveOrders(
    const Underlying& underlying) const
{
    auto it = d_activeOrders.find(underlying);
    if (it == d_activeOrders.end())
    {
        return std::nullopt;
    }
    return std::cref(it->second);
}

void OrderBook::markOrderAsFulfilled(OrderPtr completedOrder, double matchedPrice)
{
    completedOrder->matched(true);
    // match price may not be equal to initial order price
    completedOrder->matchedPrice(matchedPrice);

    removeOrderFromBook(completedOrder);

    // only remove from prices set if it's the last order left at
    // that price
    auto dequeOptional = getOrdersDequeAtPrice(completedOrder);
    if (dequeOptional && dequeOptional->get().empty())
    {
        if (completedOrder->marketSide() == MarketSide::Bid)
        {
            setBidPricesAtPriceLevel(completedOrder).erase(completedOrder->price());
        }
        else
        {
            setAskPricesAtPriceLevel(completedOrder).erase(completedOrder->price());
        }
    }
}

}  // namespace solstice::matching
