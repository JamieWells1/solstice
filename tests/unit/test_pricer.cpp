#include <gtest/gtest.h>
#include <order.h>
#include <order_book.h>
#include <pricer.h>

#include <memory>

using namespace solstice::pricing;
using namespace solstice;

class PricerTest : public ::testing::Test
{
   protected:
    std::shared_ptr<matching::OrderBook> orderBook;
    std::shared_ptr<Pricer> pricer;

    void SetUp() override
    {
        orderBook = std::make_shared<matching::OrderBook>();
        pricer = std::make_shared<Pricer>(orderBook);

        // Initialize the underlyings pool
        std::vector<Equity> pool = {Equity::AAPL};
        d_underlyingsPool<Equity> = pool;
        d_underlyingsPoolInitialised<Equity> = true;

        orderBook->initialiseBookAtUnderlyings<Equity>();
        orderBook->addEquitiesToDataMap();
    }

    void TearDown() override
    {
        d_underlyingsPool<Equity> = {};
        d_underlyingsPoolInitialised<Equity> = false;
    }
};

TEST_F(PricerTest, InitialSpreadIsSet)
{
    auto& data = orderBook->getPriceData(Equity::AAPL);

    EXPECT_GT(data.lastPrice(), 0);
    EXPECT_GE(data.demandFactor(), -1.0);
    EXPECT_LE(data.demandFactor(), 1.0);
}

TEST_F(PricerTest, FirstPriceCalculationInitializesSpread)
{
    auto& data = orderBook->getPriceData(Equity::AAPL);

    double price = pricer->calculatePrice(Equity::AAPL, MarketSide::Bid);

    EXPECT_GT(price, 0);
    EXPECT_GT(data.highestBid(), 0);
    EXPECT_GT(data.lowestAsk(), 0);
    EXPECT_GT(data.lowestAsk(), data.highestBid());
}

TEST_F(PricerTest, BidPricesAreReasonable)
{
    int pricesAtOne = 0;
    int pricesNegative = 0;
    double minPrice = 1000000;
    double maxPrice = 0;

    for (int i = 0; i < 100; i++)
    {
        double price = pricer->calculatePrice(Equity::AAPL, MarketSide::Bid);

        if (price == 1.0) pricesAtOne++;
        if (price < 0) pricesNegative++;

        minPrice = std::min(minPrice, price);
        maxPrice = std::max(maxPrice, price);
    }

    auto& data = orderBook->getPriceData(Equity::AAPL);

    EXPECT_EQ(pricesNegative, 0) << "No prices should be negative";
    EXPECT_LT(pricesAtOne, 50) << "Less than 50% should be at minimum price";
    EXPECT_GT(minPrice, 0) << "All prices should be positive";
}

TEST_F(PricerTest, AskPricesAreReasonable)
{
    int pricesAtOne = 0;
    int pricesNegative = 0;
    double minPrice = 1000000;
    double maxPrice = 0;

    for (int i = 0; i < 100; i++)
    {
        double price = pricer->calculatePrice(Equity::AAPL, MarketSide::Ask);

        if (price == 1.0) pricesAtOne++;
        if (price < 0) pricesNegative++;

        minPrice = std::min(minPrice, price);
        maxPrice = std::max(maxPrice, price);
    }

    auto& data = orderBook->getPriceData(Equity::AAPL);

    EXPECT_EQ(pricesNegative, 0) << "No prices should be negative";
    EXPECT_LT(pricesAtOne, 50) << "Less than 50% should be at minimum price";
    EXPECT_GT(minPrice, 0) << "All prices should be positive";
}

TEST_F(PricerTest, SpreadRemainsValid)
{
    auto& data = orderBook->getPriceData(Equity::AAPL);

    for (int i = 0; i < 50; i++)
    {
        pricer->calculatePrice(Equity::AAPL, MarketSide::Bid);
        pricer->calculatePrice(Equity::AAPL, MarketSide::Ask);
    }

    double spread = data.lowestAsk() - data.highestBid();

    EXPECT_GT(data.highestBid(), 0);
    EXPECT_GT(data.lowestAsk(), 0);
    EXPECT_GE(spread, 0) << "Spread should never be negative";
}

TEST_F(PricerTest, QuantityCalculationIsValid)
{
    auto& data = orderBook->getPriceData(Equity::AAPL);

    for (int i = 0; i < 10; i++)
    {
        pricer->calculatePrice(Equity::AAPL, MarketSide::Bid);
    }

    double price = pricer->calculatePrice(Equity::AAPL, MarketSide::Bid);
    double quantity = pricer->calculateQnty(Equity::AAPL, MarketSide::Bid, price);

    EXPECT_GT(quantity, 0) << "Quantity should be positive";
    EXPECT_LT(quantity, 1000000) << "Quantity should be reasonable";
}

TEST_F(PricerTest, PriceImplWithKnownValues)
{
    double lowestAsk = 100.0;
    double highestBid = 99.0;
    double demandFactor = 0.5;

    for (int i = 0; i < 10; i++)
    {
        double price =
            pricer->calculatePriceImpl(MarketSide::Bid, lowestAsk, highestBid, demandFactor);

        EXPECT_GT(price, 0) << "Price should be positive";
        EXPECT_NE(price, 1.0) << "Price shouldn't be clamped to 1.0 with valid spread";
    }

    for (int i = 0; i < 10; i++)
    {
        double price =
            pricer->calculatePriceImpl(MarketSide::Ask, lowestAsk, highestBid, demandFactor);

        EXPECT_GT(price, 0) << "Price should be positive";
        EXPECT_NE(price, 1.0) << "Price shouldn't be clamped to 1.0 with valid spread";
    }
}

TEST_F(PricerTest, QuantityDistribution)
{
    auto& data = orderBook->getPriceData(Equity::AAPL);

    int qtyAtOne = 0;
    int minQty = 1000000;
    int maxQty = 0;
    int sumQty = 0;

    for (int i = 0; i < 100; i++)
    {
        double price = 50.0;  // typical price
        int qty = pricer->calculateQnty(Equity::AAPL, MarketSide::Bid, price);

        if (qty == 1.0) qtyAtOne++;
        minQty = std::min(minQty, qty);
        maxQty = std::max(maxQty, qty);
        sumQty += qty;
    }

    data.incrementExecutions();
    data.incrementExecutions();
    data.pricesSum(100.0);
    data.pricesSumSquared(5050.0);  // sqrt(variance) ≈ small sigma

    qtyAtOne = 0;
    minQty = 1000000;
    maxQty = 0;
    sumQty = 0;

    for (int i = 0; i < 100; i++)
    {
        double price = 50.0;
        int qty = pricer->calculateQnty(Equity::AAPL, MarketSide::Bid, price);

        if (qty == 1.0) qtyAtOne++;
        minQty = std::min(minQty, qty);
        maxQty = std::max(maxQty, qty);
        sumQty += qty;
    }

    for (double price : {10.0, 50.0, 100.0, 200.0})
    {
        double qty = pricer->calculateQnty(Equity::AAPL, MarketSide::Bid, price);
        double maxQtyFormula = 10000.0 * std::abs(data.demandFactor()) / price;
    }

    qtyAtOne = 0;
    for (int i = 0; i < 100; i++)
    {
        double price = 50.0;
        double qty = pricer->calculateQnty(Equity::AAPL, MarketSide::Bid, price);
        if (qty == 1.0) qtyAtOne++;
    }

    double maxQtyLowDF = 10000.0 * 0.05 / 50.0;
}

TEST_F(PricerTest, PricesFluctuateOverTime)
{
    auto& data = orderBook->getPriceData(Equity::AAPL);

    std::set<double> uniqueBidPrices;
    std::set<double> uniqueAskPrices;

    for (int i = 0; i < 1000; i++)
    {
        double bidPrice = pricer->calculatePrice(Equity::AAPL, MarketSide::Bid);
        double askPrice = pricer->calculatePrice(Equity::AAPL, MarketSide::Ask);

        uniqueBidPrices.insert(bidPrice);
        uniqueAskPrices.insert(askPrice);
    }

    // at least 100 unique entries
    EXPECT_GT(uniqueBidPrices.size(), 100) << "Bid prices should have more variety";
    EXPECT_GT(uniqueAskPrices.size(), 100) << "Ask prices should have more variety";
}
