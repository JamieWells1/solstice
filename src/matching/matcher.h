#ifndef MATCH_H
#define MATCH_H

#include <order.h>
#include <order_book.h>
#include <types.h>

#include <expected>
#include <memory>

namespace solstice::matching
{

using OrderPtr = std::shared_ptr<Order>;

class Orchestrator;

class Matcher
{
    friend class Orchestrator;

   public:
    Matcher(std::shared_ptr<OrderBook> orderBook);

    std::expected<String, String> matchOrder(OrderPtr order, double orderMatchingPrice = -1) const;

    const std::shared_ptr<OrderBook>& orderBook() const;

   private:
    bool withinPriceRange(double price, OrderPtr order) const;
    double getDealPrice(OrderPtr firstOrder, OrderPtr secondOrder) const;
    String matchSuccessOutput(OrderPtr incomingOrder, OrderPtr matchedOrder,
                              double matchedPrice) const;

    std::shared_ptr<OrderBook> d_orderBook;
};
}  // namespace solstice::matching

#endif  // MATCH_H
