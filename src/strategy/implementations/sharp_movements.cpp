#include <market_data.h>
#include <sharp_movements.h>
#include <strategy.h>

namespace solstice::strategy
{

SharpMovements::SharpMovements(Strategy strategy, MarketData& marketData)
    : Dispatcher(std::move(strategy), marketData)
{
}

Report SharpMovements::execute()
{
    std::vector<double> highs = d_marketData.highs();
    std::vector<double> opens = d_marketData.opens();
    std::vector<double> lows = d_marketData.lows();
    std::vector<double> closes = d_marketData.closes();
    std::vector<double> timestamps = d_marketData.timestamps();
}

}  // namespace solstice::strategy
