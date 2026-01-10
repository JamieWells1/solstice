#include <broadcaster.h>
#include <config.h>
#include <gtest/gtest.h>
#include <matcher.h>
#include <orchestrator.h>
#include <order_book.h>
#include <pricer.h>

namespace solstice::matching
{

class OrchestratorFixture : public ::testing::Test
{
   protected:
    Config config = Config::instance().value();
    std::shared_ptr<OrderBook> orderBook;
    std::shared_ptr<Matcher> matcher;
    std::shared_ptr<pricing::Pricer> pricer;
    std::unique_ptr<Orchestrator> orchestrator;
    std::optional<broadcaster::Broadcaster> broadcaster;

    void SetUp() override
    {
        config.ordersToGenerate(100);
        config.usePricer(true);

        orderBook = std::make_shared<OrderBook>();
        matcher = std::make_shared<Matcher>(orderBook);
        pricer = std::make_shared<pricing::Pricer>(orderBook);

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

TEST(OrchestratorTests, StartSucceeds)
{
    std::optional<broadcaster::Broadcaster> broadcaster;
    auto result = Orchestrator::start(broadcaster);
    if (!result.has_value())
    {
        std::cout << "Error: " << result.error() << std::endl;
    }
    ASSERT_TRUE(result.has_value());
}

TEST_F(OrchestratorFixture, ProcessOrderWithMatchSucceeds)
{
    Orchestrator orch{config, orderBook, matcher, pricer, broadcaster};

    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());
    orch.processOrder(*bidOrder);

    auto askOrder = Order::create(2, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    bool result = orch.processOrder(*askOrder);
    EXPECT_TRUE(result);
}

TEST_F(OrchestratorFixture, ProcessOrderWithoutMatchFails)
{
    Orchestrator orch{config, orderBook, matcher, pricer, broadcaster};

    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());

    bool result = orch.processOrder(*bidOrder);
    EXPECT_FALSE(result);
}

TEST_F(OrchestratorFixture, ProcessOrderAddsToBook)
{
    Orchestrator orch{config, orderBook, matcher, pricer, broadcaster};

    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());

    orch.processOrder(*bidOrder);

    auto deque = orderBook->getOrdersDequeAtPrice(*bidOrder);
    ASSERT_TRUE(deque.has_value());
    EXPECT_EQ(deque->get().size(), 1);
}

TEST_F(OrchestratorFixture, ProcessOrderMarksFulfilledOnMatch)
{
    Orchestrator orch{config, orderBook, matcher, pricer, broadcaster};

    auto bidOrder = Order::create(1, Equity::AAPL, 100.0, 10.0, MarketSide::Bid);
    ASSERT_TRUE(bidOrder.has_value());
    orch.processOrder(*bidOrder);

    auto askOrder = Order::create(2, Equity::AAPL, 100.0, 10.0, MarketSide::Ask);
    ASSERT_TRUE(askOrder.has_value());

    orch.processOrder(*askOrder);

    EXPECT_TRUE((*askOrder)->matched());
}

}  // namespace solstice::matching
