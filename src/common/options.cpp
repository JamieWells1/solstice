#include <asset_class.h>
#include <greeks.h>
#include <options.h>
#include <order.h>
#include <pricer.h>
#include <time_point.h>
#include <types.h>

namespace solstice
{

OptionOrder::OptionOrder(int uid, Underlying underlying, double price, int qnty,
                         MarketSide marketSide, TimePoint timeOrderPlaced, double strike,
                         OptionType optionType, String expiry, double delta, double gamma,
                         double theta, double vega)
    : Order(uid, underlying, price, qnty, marketSide, timeOrderPlaced),
      d_strike(strike),
      d_optionType(optionType),
      d_delta(delta),
      d_gamma(gamma),
      d_theta(theta),
      d_vega(vega)
{
}

std::expected<std::shared_ptr<OptionOrder>, String> OptionOrder::create(
    int uid, Underlying underlying, double price, int qnty, MarketSide marketSide,
    TimePoint timeOrderPlaced, double strike, OptionType optionType, String expiry, double delta,
    double gamma, double theta, double vega)
{
    TimePoint d_timeOrderPlaced = timeNow();

    auto isOrderValid = validateOrderAttributes(price, qnty, timeOrderPlaced);
    if (!isOrderValid)
    {
        return std::unexpected(isOrderValid.error());
    }

    auto optionOrder = std::shared_ptr<OptionOrder>(
        new (std::nothrow) OptionOrder{uid, underlying, price, qnty, marketSide, timeOrderPlaced,
                                       strike, optionType, expiry, delta, gamma, theta, vega});

    return optionOrder;
}

std::expected<std::shared_ptr<OptionOrder>, String> OptionOrder::createWithPricer(
    std::shared_ptr<pricing::Pricer> pricer, int uid, Underlying assetClass)
{
    auto optionData = pricer->computeOptionData(assetClass);
    auto greeks = pricer->computeGreeks(optionData);

    return OptionOrder::create(uid, assetClass, optionData.price(), optionData.qnty(),
                               optionData.marketSide(), timeNow(), optionData.strike(),
                               optionData.optionType(), optionData.expiry(), greeks.delta(),
                               greeks.gamma(), greeks.theta(), greeks.vega());
}

std::expected<std::shared_ptr<OptionOrder>, String> OptionOrder::createWithRandomValues(
    Config cfg, int uid, Underlying underlying)
{
    auto data = Random::generateOptionData(cfg);
    auto greeks = Random::generateGreeks(data);

    return OptionOrder::create(uid, underlying, data.price(), data.qnty(), data.marketSide(),
                               timeNow(), data.strike(), data.optionType(), data.expiry(),
                               greeks.delta(), greeks.gamma(), greeks.theta(), greeks.vega());
}

}  // namespace solstice
