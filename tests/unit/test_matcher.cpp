#include <gtest/gtest.h>
#include <matcher.h>
#include <options.h>
#include <order_book.h>

namespace solstice::matching
{

class MatcherFixture : public ::testing::Test
{
   protected:
    std::shared_ptr<OrderBook> orderBook;
    std::shared_ptr<Matcher> matcher;

    void SetUp() override
    {
        orderBook = std::make_shared<OrderBook>();
        matcher = std::make_shared<Matcher>(orderBook);

        std::vector<Equity> pool = {Equity::AAPL};
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

TEST_F(MatcherFixture, MatchOrderWithExactPrice)
{
    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());
    orderBook->addOrderToBook(*bidOrder);

    auto askOrder = Order::create(2, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    auto result = matcher->matchOrder(*askOrder);
    ASSERT_TRUE(result.has_value());
}

TEST_F(MatcherFixture, MatchOrderWithBidHigherThanAsk)
{
    auto bidOrder = Order::create(1, Equity::AAPL, 105.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());
    orderBook->addOrderToBook(*bidOrder);

    auto askOrder = Order::create(2, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    auto result = matcher->matchOrder(*askOrder);
    ASSERT_TRUE(result.has_value());
}

TEST_F(MatcherFixture, MatchOrderFailsWhenNoOppositeOrders)
{
    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());

    auto result = matcher->matchOrder(*bidOrder);
    ASSERT_FALSE(result.has_value());
}

TEST_F(MatcherFixture, MatchOrderFailsWhenPriceOutOfRange)
{
    auto bidOrder = Order::create(1, Equity::AAPL, 95.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());
    orderBook->addOrderToBook(*bidOrder);

    auto askOrder = Order::create(2, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    auto result = matcher->matchOrder(*askOrder);
    ASSERT_FALSE(result.has_value());
}

TEST_F(MatcherFixture, MatchOrderWithPartialFill)
{
    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 5.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());
    orderBook->addOrderToBook(*bidOrder);

    auto askOrder = Order::create(2, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    auto result = matcher->matchOrder(*askOrder);
    ASSERT_FALSE(result.has_value());
}

TEST_F(MatcherFixture, MatchOrderWithMultiplePartialFills)
{
    auto bidOrder1 = Order::create(1, Equity::AAPL, 100.0, 3.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder1.has_value());
    orderBook->addOrderToBook(*bidOrder1);

    auto bidOrder2 = Order::create(2, Equity::AAPL, 100.0, 3.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder2.has_value());
    orderBook->addOrderToBook(*bidOrder2);

    auto bidOrder3 = Order::create(3, Equity::AAPL, 100.0, 4.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder3.has_value());
    orderBook->addOrderToBook(*bidOrder3);

    auto askOrder = Order::create(4, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    auto result = matcher->matchOrder(*askOrder);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*askOrder)->outstandingQnty(), 0);
}

TEST_F(MatcherFixture, MatchOrderWithIncomingOrderLargerThanResting)
{
    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 15.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());
    orderBook->addOrderToBook(*bidOrder);

    auto askOrder = Order::create(2, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    auto result = matcher->matchOrder(*askOrder);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*askOrder)->outstandingQnty(), 0);
    EXPECT_EQ((*bidOrder)->outstandingQnty(), 5.0);
}

class OptionMatcherFixture : public ::testing::Test
{
   protected:
    std::shared_ptr<OrderBook> orderBook;
    std::shared_ptr<Matcher> matcher;

    void SetUp() override
    {
        orderBook = std::make_shared<OrderBook>();
        matcher = std::make_shared<Matcher>(orderBook);

        std::vector<Option> optionPool = {Option::AAPL_JUN26_C};
        d_underlyingsPool<Option> = optionPool;
        d_underlyingsPoolInitialised<Option> = true;
        orderBook->initialiseBookAtUnderlyings<Option>();
    }

    void TearDown() override
    {
        d_underlyingsPool<Option> = {};
        d_underlyingsPoolInitialised<Option> = false;
    }
};

TEST_F(OptionMatcherFixture, OptionsMatchWhenAllAttributesMatch)
{
    auto bidOption = OptionOrder::create(1, Option::AAPL_JUN26_C, 5.0, 10, MarketSide::Bid,
                                         timeNow(), 150.0, OptionType::Call, 0.5);
    ASSERT_TRUE(bidOption.has_value());
    orderBook->addOrderToBook(*bidOption);

    auto askOption = OptionOrder::create(2, Option::AAPL_JUN26_C, 5.0, 10, MarketSide::Ask,
                                         timeNow(), 150.0, OptionType::Call, 0.5);
    ASSERT_TRUE(askOption.has_value());

    auto result = matcher->matchOrder(*askOption);
    ASSERT_TRUE(result.has_value());
}

TEST_F(OptionMatcherFixture, OptionsDoNotMatchWhenStrikeDiffers)
{
    auto bidOption = OptionOrder::create(1, Option::AAPL_JUN26_C, 5.0, 10, MarketSide::Bid,
                                         timeNow(), 150.0, OptionType::Call, 0.5);
    ASSERT_TRUE(bidOption.has_value());
    orderBook->addOrderToBook(*bidOption);

    auto askOption = OptionOrder::create(2, Option::AAPL_JUN26_C, 5.0, 10, MarketSide::Ask,
                                         timeNow(), 155.0, OptionType::Call, 0.5);
    ASSERT_TRUE(askOption.has_value());

    auto result = matcher->matchOrder(*askOption);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("matching strike") != String::npos);
}

TEST_F(OptionMatcherFixture, OptionsDoNotMatchWhenExpiryDiffers)
{
    auto bidOption = OptionOrder::create(1, Option::AAPL_JUN26_C, 5.0, 10, MarketSide::Bid,
                                         timeNow(), 150.0, OptionType::Call, 0.5);
    ASSERT_TRUE(bidOption.has_value());
    orderBook->addOrderToBook(*bidOption);

    auto askOption = OptionOrder::create(2, Option::AAPL_JUN26_C, 5.0, 10, MarketSide::Ask,
                                         timeNow(), 150.0, OptionType::Call, 0.75);
    ASSERT_TRUE(askOption.has_value());

    auto result = matcher->matchOrder(*askOption);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("matching strike") != String::npos);
}

TEST_F(OptionMatcherFixture, OptionsDoNotMatchWhenTypeDiffers)
{
    auto bidOption = OptionOrder::create(1, Option::AAPL_JUN26_C, 5.0, 10, MarketSide::Bid,
                                         timeNow(), 150.0, OptionType::Call, 0.5);
    ASSERT_TRUE(bidOption.has_value());
    orderBook->addOrderToBook(*bidOption);

    auto askOption = OptionOrder::create(2, Option::AAPL_JUN26_C, 5.0, 10, MarketSide::Ask,
                                         timeNow(), 150.0, OptionType::Put, 0.5);
    ASSERT_TRUE(askOption.has_value());

    auto result = matcher->matchOrder(*askOption);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("matching strike") != String::npos);
}

}  // namespace solstice::matching
