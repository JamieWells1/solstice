#ifndef ASSET_CLASS_H
#define ASSET_CLASS_H

#include <types.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <ostream>
#include <random>
#include <variant>
#include <vector>

namespace solstice
{

template <typename T>
inline std::vector<T> d_underlyingsPool;

template <typename T>
inline bool d_underlyingsPoolInitialised{false};

enum class AssetClass : uint8_t
{
    Equity,
    Future,
    Option,
    COUNT
};

static constexpr std::array<const char*, static_cast<size_t>(AssetClass::COUNT)> ASSET_CLASS_STR = {
    "Equity", "Future", "Option"};

constexpr std::array<AssetClass, static_cast<size_t>(AssetClass::COUNT)> ALL_ASSET_CLASSES = {
    AssetClass::Equity, AssetClass::Future, AssetClass::Option};

const char* to_string(AssetClass cls);

inline std::ostream& operator<<(std::ostream& os, const AssetClass& assetClass)
{
    return os << to_string(assetClass);
}

// ===================================================================
// Enum: Equity
// ===================================================================

enum class Equity : uint8_t
{
    AAPL,
    MSFT,
    GOOGL,
    AMZN,
    META,
    BLK,
    NVDA,
    AMD,
    INTC,
    QCOM,
    JPM,
    BAC,
    CRM,
    GS,
    MS,
    ORCL,
    IBM,
    TSM,
    UBER,
    LYFT,
    COUNT
};

static constexpr std::array<const char*, static_cast<size_t>(Equity::COUNT)> EQ_STR = {
    "AAPL", "MSFT", "GOOGL", "AMZN", "META", "BLK",  "NVDA", "AMD", "INTC", "QCOM",
    "JPM",  "BAC",  "CRM",   "GS",   "MS",   "ORCL", "IBM",  "TSM", "UBER", "LYFT"};

constexpr std::array<Equity, static_cast<size_t>(Equity::COUNT)> ALL_EQUITIES = {
    Equity::AAPL, Equity::MSFT, Equity::GOOGL, Equity::AMZN, Equity::META,
    Equity::BLK,  Equity::NVDA, Equity::AMD,   Equity::INTC, Equity::QCOM,
    Equity::JPM,  Equity::BAC,  Equity::CRM,   Equity::GS,   Equity::MS,
    Equity::ORCL, Equity::IBM,  Equity::TSM,   Equity::UBER, Equity::LYFT};

// ===================================================================
// Enum: Future
// ===================================================================

enum class Future : uint8_t
{
    AAPL_MAR26,
    AAPL_JUN26,
    AAPL_SEP26,
    AAPL_DEC26,

    MSFT_MAR26,
    MSFT_JUN26,
    MSFT_SEP26,
    MSFT_DEC26,

    TSLA_MAR26,
    TSLA_JUN26,
    TSLA_SEP26,
    TSLA_DEC26,

    COUNT
};

static constexpr std::array<const char*, static_cast<size_t>(Future::COUNT)> FTR_STR = {
    "AAPL_MAR26", "AAPL_JUN26", "AAPL_SEP26", "AAPL_DEC26", "MSFT_MAR26", "MSFT_JUN26",
    "MSFT_SEP26", "MSFT_DEC26", "TSLA_MAR26", "TSLA_JUN26", "TSLA_SEP26", "TSLA_DEC26",
};

constexpr std::array<Future, static_cast<size_t>(Future::COUNT)> ALL_FUTURES = {
    Future::AAPL_MAR26, Future::AAPL_JUN26, Future::AAPL_SEP26, Future::AAPL_DEC26,
    Future::MSFT_MAR26, Future::MSFT_JUN26, Future::MSFT_SEP26, Future::MSFT_DEC26,
    Future::TSLA_MAR26, Future::TSLA_JUN26, Future::TSLA_SEP26, Future::TSLA_DEC26};

// ===================================================================
// Enum: Option
// ===================================================================

enum class Option : uint8_t
{
    // AAPL Calls
    AAPL_MAR26_C,
    AAPL_JUN26_C,
    AAPL_SEP26_C,
    AAPL_DEC26_C,

    // AAPL Puts
    AAPL_MAR26_P,
    AAPL_JUN26_P,
    AAPL_SEP26_P,
    AAPL_DEC26_P,

    // MSFT Calls
    MSFT_MAR26_C,
    MSFT_JUN26_C,
    MSFT_SEP26_C,
    MSFT_DEC26_C,

    // MSFT Puts
    MSFT_MAR26_P,
    MSFT_JUN26_P,
    MSFT_SEP26_P,
    MSFT_DEC26_P,

    // TSLA Calls
    TSLA_MAR26_C,
    TSLA_JUN26_C,
    TSLA_SEP26_C,
    TSLA_DEC26_C,

    // TSLA Puts
    TSLA_MAR26_P,
    TSLA_JUN26_P,
    TSLA_SEP26_P,
    TSLA_DEC26_P,

    COUNT
};

static constexpr std::array<const char*, static_cast<size_t>(Option::COUNT)> OPT_STR = {
    "AAPL_MAR26_C", "AAPL_JUN26_C", "AAPL_SEP26_C", "AAPL_DEC26_C", "AAPL_MAR26_P", "AAPL_JUN26_P",
    "AAPL_SEP26_P", "AAPL_DEC26_P", "MSFT_MAR26_C", "MSFT_JUN26_C", "MSFT_SEP26_C", "MSFT_DEC26_C",
    "MSFT_MAR26_P", "MSFT_JUN26_P", "MSFT_SEP26_P", "MSFT_DEC26_P", "TSLA_MAR26_C", "TSLA_JUN26_C",
    "TSLA_SEP26_C", "TSLA_DEC26_C", "TSLA_MAR26_P", "TSLA_JUN26_P", "TSLA_SEP26_P", "TSLA_DEC26_P",
};

constexpr std::array<Option, static_cast<size_t>(Option::COUNT)> ALL_OPTIONS = {
    Option::AAPL_MAR26_C, Option::AAPL_JUN26_C, Option::AAPL_SEP26_C, Option::AAPL_DEC26_C,
    Option::AAPL_MAR26_P, Option::AAPL_JUN26_P, Option::AAPL_SEP26_P, Option::AAPL_DEC26_P,
    Option::MSFT_MAR26_C, Option::MSFT_JUN26_C, Option::MSFT_SEP26_C, Option::MSFT_DEC26_C,
    Option::MSFT_MAR26_P, Option::MSFT_JUN26_P, Option::MSFT_SEP26_P, Option::MSFT_DEC26_P,
    Option::TSLA_MAR26_C, Option::TSLA_JUN26_C, Option::TSLA_SEP26_C, Option::TSLA_DEC26_C,
    Option::TSLA_MAR26_P, Option::TSLA_JUN26_P, Option::TSLA_SEP26_P, Option::TSLA_DEC26_P,
};

// ===================================================================
// Type Declaration
// ===================================================================

using Underlying = std::variant<Equity, Future, Option>;

AssetClass randomAssetClass();

std::expected<Underlying, String> getUnderlying(AssetClass assetClass);

// ===================================================================
// Template Functions
// ===================================================================

template <typename T>
const char* to_string(T type)
{
    int underlying = static_cast<size_t>(type);

    if (std::is_same_v<T, Future>) return FTR_STR[underlying];

    if (std::is_same_v<T, Equity>) return EQ_STR[underlying];

    if (std::is_same_v<T, Option>) return OPT_STR[underlying];
}

template <typename T>
const std::expected<T, String> randomUnderlying()
{
    const auto& pool = d_underlyingsPool<T>;
    if (pool.empty())
    {
        return std::unexpected("Underlying pool is empty");
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, pool.size() - 1);

    return pool[dist(gen)];
}

template <typename... Types>
const char* to_string(const std::variant<Types...>& variant)
{
    return std::visit([](const auto& underlying) { return to_string(underlying); }, variant);
}

inline std::ostream& operator<<(std::ostream& os, const Underlying& underlying)
{
    return os << to_string(underlying);
}

template <typename T, typename... Types>
inline bool operator==(const std::variant<Types...>& variant, const T& value)
{
    if (const T* ptr = std::get_if<T>(&variant))
    {
        return *ptr == value;
    }
    return false;
}

template <typename T, typename... Types>
inline bool operator==(const T& value, const std::variant<Types...>& variant)
{
    return variant == value;
}

// ===================================================================
// Inline Getters/Setters
// ===================================================================

template <typename T>
inline const bool underlyingsPoolInitialised()
{
    return d_underlyingsPoolInitialised<T>;
}

template <typename T>
inline bool setUnderlyingsPoolInitialised(bool isInitialised)
{
    d_underlyingsPoolInitialised<T> = isInitialised;
    return d_underlyingsPoolInitialised<T>;
}

template <typename T>
inline const std::vector<T>& underlyingsPool()
{
    return d_underlyingsPool<T>;
}

template <typename T, std::size_t N>
inline void setUnderlyingsPool(int poolSize, const std::array<T, N>& fullSet)
{
    if (underlyingsPoolInitialised<T>()) return;

    auto& pool = d_underlyingsPool<T>;
    pool.assign(fullSet.begin(), fullSet.end());

    if (poolSize > 0 && poolSize < static_cast<int>(pool.size()))
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::shuffle(pool.begin(), pool.end(), gen);
        pool.resize(poolSize);
    }

    setUnderlyingsPoolInitialised<T>(true);
}

}  // namespace solstice

#endif  // ASSET_CLASS_H
