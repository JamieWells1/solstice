#include <gtest/gtest.h>
#include <order.h>
#include <order_book.h>

namespace solstice::matching
{

class OrderBookFixture : public ::testing::Test
{
   protected:
    std::shared_ptr<OrderBook> orderBook;

    void SetUp() override
    {
        orderBook = std::make_shared<OrderBook>();

        std::vector<Equity> pool = {Equity::AAPL, Equity::MSFT};
        d_underlyingsPool<Equity> = pool;
        d_underlyingsPoolInitialised<Equity> = true;
        orderBook->initialiseBookAtUnderlyings<Equity>();
    }

    void TearDown() override
    {
        d_underlyingsPool<Equity> = {};
        d_underlyingsPoolInitialised<Equity> = false;
    }
};

TEST_F(OrderBookFixture, AddOrderToBookSucceeds)
{
    auto order = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(order.has_value());

    orderBook->addOrderToBook(*order);

    auto deque = orderBook->getOrdersDequeAtPrice(*order);
    ASSERT_TRUE(deque.has_value());
    EXPECT_EQ(deque->get().size(), 1);
}

TEST_F(OrderBookFixture, AddMultipleOrdersAtSamePrice)
{
    auto order1 = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    auto order2 = Order::create(2, Equity::AAPL, 100.0, 15.0, MarketSide::Bid);
    ASSERT_TRUE(order1.has_value());
    ASSERT_TRUE(order2.has_value());

    orderBook->addOrderToBook(*order1);
    orderBook->addOrderToBook(*order2);

    auto deque = orderBook->getOrdersDequeAtPrice(*order1);
    ASSERT_TRUE(deque.has_value());
    EXPECT_EQ(deque->get().size(), 2);
}

TEST_F(OrderBookFixture, AddOrdersAtDifferentPrices)
{
    auto order1 = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    auto order2 = Order::create(2, Equity::AAPL, 105.0, 15.0, MarketSide::Bid);
    ASSERT_TRUE(order1.has_value());
    ASSERT_TRUE(order2.has_value());

    orderBook->addOrderToBook(*order1);
    orderBook->addOrderToBook(*order2);

    auto deque1 = orderBook->getOrdersDequeAtPrice(*order1);
    auto deque2 = orderBook->getOrdersDequeAtPrice(*order2);
    ASSERT_TRUE(deque1.has_value());
    ASSERT_TRUE(deque2.has_value());
    EXPECT_EQ(deque1->get().size(), 1);
    EXPECT_EQ(deque2->get().size(), 1);
}

TEST_F(OrderBookFixture, GetBestPriceForBid)
{
    auto askOrder1 = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    auto askOrder2 = Order::create(2, Equity::AAPL, 105.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder1.has_value());
    ASSERT_TRUE(askOrder2.has_value());

    orderBook->addOrderToBook(*askOrder1);
    orderBook->addOrderToBook(*askOrder2);

    auto bidOrder = Order::create(3, Equity::AAPL, 102.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());

    auto bestPrice = orderBook->getBestPrice(*bidOrder);
    ASSERT_TRUE(bestPrice.has_value());
    EXPECT_EQ(*bestPrice, 100.0);
}

TEST_F(OrderBookFixture, GetBestPriceForAsk)
{
    auto bidOrder1 = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    auto bidOrder2 = Order::create(2, Equity::AAPL, 95.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder1.has_value());
    ASSERT_TRUE(bidOrder2.has_value());

    orderBook->addOrderToBook(*bidOrder1);
    orderBook->addOrderToBook(*bidOrder2);

    auto askOrder = Order::create(3, Equity::AAPL, 98.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    auto bestPrice = orderBook->getBestPrice(*askOrder);
    ASSERT_TRUE(bestPrice.has_value());
    EXPECT_EQ(*bestPrice, 100.0);
}

TEST_F(OrderBookFixture, GetBestPriceFailsWhenNoOppositeOrders)
{
    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());

    auto bestPrice = orderBook->getBestPrice(*bidOrder);
    ASSERT_FALSE(bestPrice.has_value());
}

TEST_F(OrderBookFixture, GetBestPriceFailsWhenPriceOutOfRange)
{
    auto askOrder1 = Order::create(1, Equity::AAPL, 110.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder1.has_value());
    orderBook->addOrderToBook(*askOrder1);

    auto bidOrder = Order::create(2, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());

    auto bestPrice = orderBook->getBestPrice(*bidOrder);
    ASSERT_FALSE(bestPrice.has_value());
}

TEST_F(OrderBookFixture, MarkOrderAsFulfilledRemovesOrder)
{
    auto order = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(order.has_value());

    orderBook->addOrderToBook(*order);
    orderBook->markOrderAsFulfilled(*order, 100);

    EXPECT_TRUE((*order)->matched());
}

TEST_F(OrderBookFixture, MarkOrderAsFulfilledRemovesPriceWhenLastOrder)
{
    auto order = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(order.has_value());

    orderBook->addOrderToBook(*order);

    auto bestPrice = orderBook->getBestPrice(*order);

    orderBook->markOrderAsFulfilled(*order, *bestPrice);

    auto deque = orderBook->getOrdersDequeAtPrice(*order);
    ASSERT_TRUE(deque.has_value());
    EXPECT_TRUE(deque->get().empty());
}

TEST_F(OrderBookFixture, OppositeMarketSidePriceLevelMapReturnsBidsForAsk)
{
    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    auto askOrder = Order::create(2, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(bidOrder.has_value());
    ASSERT_TRUE(askOrder.has_value());

    orderBook->addOrderToBook(*bidOrder);

    auto oppositeMap = orderBook->oppositeMarketSidePriceLevelMap(*askOrder);
    EXPECT_FALSE(oppositeMap.empty());
}

TEST_F(OrderBookFixture, SameMarketSidePriceLevelMapReturnsBidsForBid)
{
    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());

    orderBook->addOrderToBook(*bidOrder);

    auto sameMap = orderBook->sameMarketSidePriceLevelMap(*bidOrder);
    EXPECT_FALSE(sameMap.empty());
}

TEST_F(OrderBookFixture, GetPriceLevelOppositeOrdersSucceeds)
{
    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());
    orderBook->addOrderToBook(*bidOrder);

    auto askOrder = Order::create(2, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    auto resultRaw = orderBook->getPriceLevelOppositeOrders(*askOrder, 100.0);
    ASSERT_TRUE(resultRaw.has_value());

    auto result = (*resultRaw).get();
    EXPECT_EQ(result.size(), 1);
}

TEST_F(OrderBookFixture, GetPriceLevelOppositeOrdersFailsWhenNoOrders)
{
    auto askOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    auto result = orderBook->getPriceLevelOppositeOrders(*askOrder, 100.0);
    ASSERT_FALSE(result.has_value());
}

TEST_F(OrderBookFixture, MultipleUnderlyingsSupported)
{
    auto aaplOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    auto msftOrder = Order::create(2, Equity::MSFT, 200.0, 15.0, MarketSide::Bid);
    ASSERT_TRUE(aaplOrder.has_value());
    ASSERT_TRUE(msftOrder.has_value());

    orderBook->addOrderToBook(*aaplOrder);
    orderBook->addOrderToBook(*msftOrder);

    auto aaplDeque = orderBook->getOrdersDequeAtPrice(*aaplOrder);
    auto msftDeque = orderBook->getOrdersDequeAtPrice(*msftOrder);
    ASSERT_TRUE(aaplDeque.has_value());
    ASSERT_TRUE(msftDeque.has_value());
    EXPECT_EQ(aaplDeque->get().size(), 1);
    EXPECT_EQ(msftDeque->get().size(), 1);
}

TEST_F(OrderBookFixture, TransactionsInitiallyEmpty)
{
    EXPECT_TRUE(orderBook->transactions().empty());
}

}  // namespace solstice::matching
