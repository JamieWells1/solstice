#include <asset_class.h>
#include <gtest/gtest.h>

namespace solstice
{

template <typename T>
void resetGlobalStateBeforeTest()
{
    d_underlyingsPool<T> = {};
    d_underlyingsPoolInitialised<T> = false;
}

template <typename T>
void resetGlobalState()
{
    d_underlyingsPool<T> = {};
    d_underlyingsPoolInitialised<T> = false;
}

template <typename... Ts>
void resetGlobalStateAllBeforeTest()
{
    (resetGlobalStateBeforeTest<Ts>(), ...);
}

template <typename... Ts>
void resetGlobalStateAll()
{
    (resetGlobalState<Ts>(), ...);
}

class UnderlyingTests : public ::testing::Test
{
   protected:
    void SetUp() override { resetGlobalStateAllBeforeTest<Equity, Future, Option>(); }

    void TearDown() override { resetGlobalStateAll<Equity, Future, Option>(); }
};

TEST_F(UnderlyingTests, UnderlyingToString)
{
    ASSERT_TRUE(to_string(Equity::AAPL) == std::string("AAPL"));
}

TEST_F(UnderlyingTests, UnderlyingPoolPopulates)
{
    setUnderlyingsPool(10, ALL_EQUITIES);
    ASSERT_TRUE(!underlyingsPool<Equity>().empty());
}

TEST_F(UnderlyingTests, ValidRandomUnderlying)
{
    setUnderlyingsPool(10, ALL_EQUITIES);

    Equity result = *randomUnderlying<Equity>();

    ASSERT_TRUE(std::find(ALL_EQUITIES.begin(), ALL_EQUITIES.end(), result) != ALL_EQUITIES.end());

    ASSERT_TRUE((std::is_same_v<std::remove_cv_t<decltype(result)>, Equity>));
}

TEST_F(UnderlyingTests, randomUnderlyingFailsWhenPoolEmpty)
{
    auto result = randomUnderlying<Equity>();
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().find("Underlying pool is empty") != std::string::npos);
}

TEST_F(UnderlyingTests, SetUnderlyingsPoolWithFullSet)
{
    setUnderlyingsPool(-1, ALL_EQUITIES);
    auto pool = underlyingsPool<Equity>();
    EXPECT_EQ(pool.size(), ALL_EQUITIES.size());
}

TEST_F(UnderlyingTests, SetUnderlyingsPoolWithLimitedSize)
{
    setUnderlyingsPool(5, ALL_EQUITIES);
    auto pool = underlyingsPool<Equity>();
    EXPECT_EQ(pool.size(), 5);
}

TEST_F(UnderlyingTests, SetUnderlyingsPoolOnlyInitializesOnce)
{
    setUnderlyingsPool(5, ALL_EQUITIES);
    auto pool1 = underlyingsPool<Equity>();

    setUnderlyingsPool(10, ALL_EQUITIES);
    auto pool2 = underlyingsPool<Equity>();

    EXPECT_EQ(pool1.size(), pool2.size());
    EXPECT_EQ(pool2.size(), 5);
}

TEST_F(UnderlyingTests, underlyingsPoolInitialisedReturnsTrueAfterInit)
{
    ASSERT_FALSE(underlyingsPoolInitialised<Equity>());
    setUnderlyingsPool(10, ALL_EQUITIES);
    ASSERT_TRUE(underlyingsPoolInitialised<Equity>());
}

TEST_F(UnderlyingTests, EquityUnderlyingPoolWorks)
{
    setUnderlyingsPool(5, ALL_EQUITIES);
    auto pool = underlyingsPool<Equity>();
    EXPECT_EQ(pool.size(), 5);

    auto result = randomUnderlying<Equity>();
    ASSERT_TRUE(result.has_value());
}

TEST_F(UnderlyingTests, FutureUnderlyingPoolWorks)
{
    setUnderlyingsPool(5, ALL_FUTURES);
    auto pool = underlyingsPool<Future>();
    EXPECT_EQ(pool.size(), 5);

    auto result = randomUnderlying<Future>();
    ASSERT_TRUE(result.has_value());
}

TEST_F(UnderlyingTests, OptionUnderlyingPoolWorks)
{
    setUnderlyingsPool(5, ALL_OPTIONS);
    auto pool = underlyingsPool<Option>();
    EXPECT_EQ(pool.size(), 5);

    auto result = randomUnderlying<Option>();
    ASSERT_TRUE(result.has_value());
}

}  // namespace solstice
