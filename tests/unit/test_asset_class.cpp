#include <asset_class.h>
#include <gtest/gtest.h>
#include <order.h>

#include <algorithm>
#include <type_traits>

// only stateless helper functions live in this file, hence the .cpp import
#include <asset_class.cpp>

namespace solstice
{

TEST(AssetClassTests, AssetClassConvertsToString)
{
    ASSERT_TRUE(to_string(AssetClass::Equity) == std::string("Equity"));
}

TEST(AssetClassTests, ValidRandomAssetClass)
{
    auto result = randomAssetClass();
    ASSERT_TRUE(std::find(ALL_ASSET_CLASSES.begin(), ALL_ASSET_CLASSES.end(), result) !=
                ALL_ASSET_CLASSES.end());

    ASSERT_TRUE((std::is_same_v<decltype(result), AssetClass>));
}

TEST(AssetClassTests, EquityConvertsToString)
{
    ASSERT_TRUE(std::string(to_string(Equity::AAPL)) == "AAPL");
    ASSERT_TRUE(std::string(to_string(Equity::MSFT)) == "MSFT");
    ASSERT_TRUE(std::string(to_string(Equity::GOOGL)) == "GOOGL");
}

TEST(AssetClassTests, FutureConvertsToString)
{
    ASSERT_TRUE(std::string(to_string(Future::AAPL_MAR26)) == "AAPL_MAR26");
    ASSERT_TRUE(std::string(to_string(Future::MSFT_JUN26)) == "MSFT_JUN26");
}

TEST(AssetClassTests, UnderlyingVariantConvertsToString)
{
    Underlying eqVariant = Equity::AAPL;
    ASSERT_TRUE(std::string(to_string(eqVariant)) == "AAPL");

    Underlying ftrVariant = Future::TSLA_DEC26;
    ASSERT_TRUE(std::string(to_string(ftrVariant)) == "TSLA_DEC26");
}

TEST(AssetClassTests, UnderlyingVariantEquality)
{
    Underlying variant = Equity::MSFT;
    ASSERT_TRUE(variant == Equity::MSFT);
    ASSERT_TRUE(Equity::MSFT == variant);
    ASSERT_FALSE(variant == Equity::AAPL);
    ASSERT_FALSE(variant == Future::AAPL_MAR26);
}

}  // namespace solstice
