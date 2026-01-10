#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <account.h>
#include <market_data.h>
#include <strategy.h>

#include <memory>

namespace solstice::strategy
{

struct Report
{
    Report(int candlesAnalysed, int tradesCompleted, int longTrades, int shortTrades, double pnl,
           int winningTrades, int losingTrades);

    int d_candlesAnalysed;
    int d_tradesCompleted;
    int d_longTrades;
    int d_shortTrades;

    double d_pnl;
    int d_winningTrades;
    int d_losingTrades;
};

class Dispatcher
{
   public:
    template <typename T>
    static std::unique_ptr<T> constructStrategy(Strategy strategy, MarketData& marketData);

    virtual Report execute() = 0;
    virtual ~Dispatcher() = default;

   protected:
    Dispatcher(Strategy strategy, MarketData& marketData);

    Account d_account;
    Strategy d_strategy;
    MarketData d_marketData;
};

// strategy types
template <typename T>
std::unique_ptr<T> Dispatcher::constructStrategy(Strategy strategy, MarketData& marketData)
{
    switch (strategy)
    {
        case Strategy::SharpMovements:
            return std::make_unique<T>(std::move(strategy), marketData);
        default:
            return nullptr;
    }
}

}  // namespace solstice::strategy

#endif  // DISPATCHER_H
