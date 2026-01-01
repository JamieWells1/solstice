#include <options.h>
#include <order.h>
#include <pricer.h>
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

std::expected<std::shared_ptr<Order>, String> Order::createWithPricer(
    std::shared_ptr<pricing::Pricer> pricer, int uid, Underlying underlying)
{
    pricing::PricerDepOptionData data = pricer->computeOrderData(underlying);

    return Order::create(uid, underlying, data.price(), data.qnty(), data.marketSide());
}

std::expected<std::shared_ptr<Order>, String> Order::createWithRandomValues(Config cfg, int uid,
                                                                            Underlying underlying)
{
    int price = Order::getRandomPrice(cfg.minPrice(), cfg.maxPrice());
    int qnty = Order::getRandomQnty(cfg.minQnty(), cfg.maxQnty());
    MarketSide mktSide = Order::getRandomMarketSide();

    return Order::create(uid, underlying, price, qnty, mktSide);
}

}  // namespace solstice
