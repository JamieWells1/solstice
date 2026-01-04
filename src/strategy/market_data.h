#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <types.h>

#include <unordered_map>
#include <vector>

namespace solstice
{
using RawMarketData = std::unordered_map<String, std::vector<double>>;
}

namespace solstice::strategy
{

struct MarketData
{
   public:
    static MarketData mapRawInput(RawMarketData& inputData);

    const std::vector<double>& opens() const;
    const std::vector<double>& highs() const;
    const std::vector<double>& lows() const;
    const std::vector<double>& closes() const;
    const std::vector<double>& volume() const;
    const std::vector<double>& timestamps() const;

   private:
    MarketData(std::vector<double> opens, std::vector<double> highs, std::vector<double> lows,
               std::vector<double> closes, std::vector<double> volume,
               std::vector<double> timestamps);

    const std::vector<double> d_opens;
    const std::vector<double> d_highs;
    const std::vector<double> d_lows;
    const std::vector<double> d_closes;
    const std::vector<double> d_volume;
    const std::vector<double> d_timestamps;
};

}  // namespace solstice::strategy

#endif  // MARKET_DATA_H
