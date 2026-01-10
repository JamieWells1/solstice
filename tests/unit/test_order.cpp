#include <asset_class.h>
#include <gtest/gtest.h>
#include <market_side.h>
#include <order.h>

namespace solstice
{

TEST(OrderTests, ValidOrderSucceeds)
{
    auto result = Order::create(0, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value()->underlying(), Equity::AAPL);
}

TEST(OrderTests, NegativePriceFails)
{
    auto result = Order::create(0, Equity::AAPL, -10.0, 10.0, MarketSide::Ask);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("Invalid price") != String::npos);
}

TEST(OrderTests, NegativeQntyFails)
{
    auto result = Order::create(0, Equity::AAPL, 100.0, -10.0, MarketSide::Ask);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("Invalid quantity") != String::npos);
}

TEST(OrderTests, OrderHasCorrectUid)
{
    auto result = Order::create(42, Equity::MSFT, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value()->uid(), 42);
}

TEST(OrderTests, OrderHasCorrectUnderlying)
{
    auto result = Order::create(0, Equity::GOOGL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value()->underlying() == Equity::GOOGL);
}

TEST(OrderTests, OrderHasCorrectPrice)
{
    auto result = Order::create(0, Equity::AAPL, 123.45, 10.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value()->price(), 123.45);
}

TEST(OrderTests, OrderHasCorrectQuantity)
{
    auto result = Order::create(0, Equity::AAPL, 100.0, 25.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value()->qnty(), 25.0);
}

TEST(OrderTests, OrderHasCorrectSide)
{
    auto bidResult = Order::create(0, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidResult.has_value());
    EXPECT_EQ(bidResult.value()->marketSide(), MarketSide::Bid);

    auto askResult = Order::create(0, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askResult.has_value());
    EXPECT_EQ(askResult.value()->marketSide(), MarketSide::Ask);
}

TEST(OrderTests, OrderInitiallyNotComplete)
{
    auto result = Order::create(0, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result.value()->matched());
}

TEST(OrderTests, OrderOutstandingQntyInitiallyEqualsQnty)
{
    auto result = Order::create(0, Equity::AAPL, 100.0, 15.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value()->outstandingQnty(), 15.0);
}

TEST(OrderTests, CanUpdateOutstandingQnty)
{
    auto result = Order::create(0, Equity::AAPL, 100.0, 15.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
    result.value()->outstandingQnty(5.0);
    EXPECT_EQ(result.value()->outstandingQnty(), 5.0);
}

TEST(OrderTests, CanMarkOrderAsComplete)
{
    auto result = Order::create(0, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
    result.value()->matched(true);
    EXPECT_TRUE(result.value()->matched());
}

TEST(OrderTests, TimeOrderFulfilledFailsWhenNotComplete)
{
    auto result = Order::create(0, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
    auto timeResult = result.value()->timeOrderFulfilled();
    ASSERT_FALSE(timeResult.has_value());
}

TEST(OrderTests, MarketSideStringReturnsCorrectValue)
{
    auto bidResult = Order::create(0, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidResult.has_value());
    EXPECT_EQ(bidResult.value()->marketSideString(), "Bid");

    auto askResult = Order::create(0, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askResult.has_value());
    EXPECT_EQ(askResult.value()->marketSideString(), "Ask");
}

TEST(OrderTests, ZeroPriceIsValid)
{
    auto result = Order::create(0, Equity::AAPL, 0.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
}

TEST(OrderTests, ZeroQntyIsValid)
{
    auto result = Order::create(0, Equity::AAPL, 100.0, 0.0, MarketSide::Bid);
    ASSERT_TRUE(result.has_value());
}

}  // namespace solstice
