#include <market_side.h>
#include <matcher.h>
#include <order.h>
#include <order_book.h>
#include <types.h>

#include <expected>
#include <iostream>
#include <memory>

namespace solstice::matching
{

bool Matcher::withinPriceRange(double price, OrderPtr order) const
{
    if (order->marketSide() == MarketSide::Bid)
    {
        return price > order->price() ? false : true;
    }
    return price < order->price() ? false : true;
}

double Matcher::getDealPrice(OrderPtr firstOrder, OrderPtr secondOrder) const
{
    if (firstOrder->price() == secondOrder->price())
    {
        // doesn't matter which price is returned as they are equal
        return firstOrder->price();
    }

    OrderPtr bid = firstOrder->marketSide() == MarketSide::Bid ? firstOrder : secondOrder;
    OrderPtr ask = firstOrder == bid ? secondOrder : firstOrder;

    // always return price of resting order
    if (ask->timeOrderPlaced() > bid->timeOrderPlaced())
    {
        return ask->price();
    }

    if (bid->timeOrderPlaced() > ask->timeOrderPlaced())
    {
        return bid->price();
    }

    // tiebreaker - uid as this is based on position in book
    return bid->uid() > ask->uid() ? ask->price() : bid->price();
}

String Matcher::matchSuccessOutput(OrderPtr incomingOrder, OrderPtr matchedOrder,
                                   double matchedPrice) const
{
    const double dealPrice = getDealPrice(incomingOrder, matchedOrder);

    std::ostringstream oss;

    // Incoming order
    oss << "Order: " << incomingOrder->uid() << " | Asset class: " << incomingOrder->assetClass()
        << " | Status: Matched"
        << " | Matched with: " << matchedOrder->uid()
        << " | Side: " << incomingOrder->marketSideString()
        << " | Ticker: " << to_string(incomingOrder->underlying()) << " | Price: $"
        << incomingOrder->price() << " | Qnty: " << incomingOrder->qnty()
        << " | Remaining Qnty: " << incomingOrder->outstandingQnty();

    if (incomingOrder->outstandingQnty() == 0)
    {
        oss << " [FULFILLED]";
    }
    oss << "\n";

    // Matched order
    oss << "Order: " << matchedOrder->uid() << " | Asset class: " << incomingOrder->assetClass()
        << " | Status: Matched"
        << " | Matched with: " << incomingOrder->uid()
        << " | Side: " << matchedOrder->marketSideString()
        << " | Ticker: " << to_string(matchedOrder->underlying()) << " | Price: $"
        << matchedOrder->price() << " | Qnty: " << matchedOrder->qnty()
        << " | Remaining Qnty: " << matchedOrder->outstandingQnty();

    if (matchedOrder->outstandingQnty() == 0)
    {
        oss << " [FULFILLED]";
    }
    oss << "\n\n";

    return oss.str();
}

std::expected<String, String> Matcher::matchOrder(OrderPtr incomingOrder,
                                                  double orderMatchingPrice) const
{
    double bestPrice = orderMatchingPrice;

    if (orderMatchingPrice == -1)
    {
        auto bestPriceAvailable = d_orderBook->getBestPrice(incomingOrder);
        if (!bestPriceAvailable)
        {
            return std::unexpected(bestPriceAvailable.error());
        }

        bestPrice = *bestPriceAvailable;
    }

    std::map<double, std::deque<OrderPtr>>& priceLevelMap =
        d_orderBook->oppositeMarketSidePriceLevelMap(incomingOrder);

    auto it = priceLevelMap.find(bestPrice);
    if (it == priceLevelMap.end())
    {
        return std::unexpected("Insufficient orders available to fulfill incoming order\n");
    }

    auto ordersResult = d_orderBook->getPriceLevelOppositeOrders(incomingOrder, bestPrice);

    if (!ordersResult)
    {
        return std::unexpected(ordersResult.error());
    }

    std::deque<OrderPtr>& ordersAtBestPrice = *ordersResult;
    OrderPtr bestOrder = ordersAtBestPrice.at(0);

    if (ordersAtBestPrice.size() == 1 && bestOrder->uid() == incomingOrder->uid())
    {
        return std::unexpected("Orders cannot match themselves\n");
    }
    if (bestOrder->outstandingQnty() < incomingOrder->outstandingQnty())
    {
        double transactionQnty = bestOrder->outstandingQnty();
        incomingOrder->outstandingQnty(incomingOrder->outstandingQnty() - transactionQnty);
        bestOrder->outstandingQnty(0);

        const String partialMatchResult = matchSuccessOutput(incomingOrder, bestOrder, bestPrice);

        d_orderBook->markOrderAsFulfilled(bestOrder, bestPrice);

        if (!ordersAtBestPrice.empty())
        {
            auto result = matchOrder(incomingOrder, bestPrice);
            if (result.has_value())
            {
                return partialMatchResult + *result;
            }
            return result;
        }
        else
        {
            auto nextIt = std::next(it);
            if (nextIt == priceLevelMap.end())
            {
                return std::unexpected("Insufficient orders available to fulfill incoming order\n");
            }

            const double nextBestPrice = nextIt->first;

            if (!withinPriceRange(nextBestPrice, incomingOrder))
            {
                return std::unexpected("All other orders out of price range\n");
            }

            auto result = matchOrder(incomingOrder, nextBestPrice);
            if (result.has_value())
            {
                return partialMatchResult + *result;
            }
            return result;
        }
    }
    else if (bestOrder->outstandingQnty() == incomingOrder->outstandingQnty())
    {
        bestOrder->outstandingQnty(0);
        incomingOrder->outstandingQnty(0);

        const String& finalMatchResult = matchSuccessOutput(incomingOrder, bestOrder, bestPrice);

        d_orderBook->markOrderAsFulfilled(bestOrder, bestPrice);
        d_orderBook->markOrderAsFulfilled(incomingOrder, bestPrice);

        return finalMatchResult;
    }
    else if (bestOrder->outstandingQnty() > incomingOrder->outstandingQnty())
    {
        double transactionQnty = incomingOrder->outstandingQnty();

        bestOrder->outstandingQnty(bestOrder->outstandingQnty() - transactionQnty);
        incomingOrder->outstandingQnty(0);

        const String& finalMatchResult = matchSuccessOutput(incomingOrder, bestOrder, bestPrice);

        d_orderBook->markOrderAsFulfilled(incomingOrder, bestPrice);

        return finalMatchResult;
    }

    return std::unexpected("An error occured when fetching order quantity\n");
}

Matcher::Matcher(std::shared_ptr<OrderBook> orderBook) : d_orderBook(orderBook) {}

const std::shared_ptr<OrderBook>& Matcher::orderBook() const { return d_orderBook; }

}  // namespace solstice::matching
