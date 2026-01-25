#include <config.h>
#include <gtest/gtest.h>

namespace solstice
{

TEST(ConfigTests, ValidConfigSucceeds)
{
    auto resultRaw = Config::instance();
    ASSERT_TRUE(resultRaw);
    auto result = *resultRaw;

    result.maxPrice(100);

    EXPECT_EQ(result.maxPrice(), 100);
}

TEST(ConfigTests, CanModifyAllFields)
{
    auto resultRaw = Config::instance();
    ASSERT_TRUE(resultRaw);
    auto result = *resultRaw;

    result.logLevel(LogLevel::DEBUG);
    result.assetClass(AssetClass::Equity);
    result.ordersToGenerate(5000);
    result.underlyingPoolCount(25);
    result.minQnty(5);
    result.maxQnty(50);
    result.minPrice(100.0);
    result.maxPrice(200.0);
    result.usePricer(true);
    result.broadcastInterval(20);

    EXPECT_EQ(result.logLevel(), LogLevel::DEBUG);
    EXPECT_EQ(result.assetClass(), AssetClass::Equity);
    EXPECT_EQ(result.ordersToGenerate(), 5000);
    EXPECT_EQ(result.underlyingPoolCount(), 25);
    EXPECT_EQ(result.minQnty(), 5);
    EXPECT_EQ(result.maxQnty(), 50);
    EXPECT_EQ(result.minPrice(), 100.0);
    EXPECT_EQ(result.maxPrice(), 200.0);
    EXPECT_EQ(result.usePricer(), true);
    EXPECT_EQ(result.broadcastInterval(), 20);
}

}  // namespace solstice
