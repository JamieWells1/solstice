#include <asset_class.h>
#include <greeks.h>
#include <market_side.h>
#include <options.h>
#include <order.h>
#include <pricer.h>
#include <time_point.h>
#include <types.h>

#include <format>

namespace solstice
{

Resolution<Equity> extractUnderlyingEquity(Option optionTicker)
{
    String optionString = to_string(optionTicker);

    size_t firstUnderscore = optionString.find('_');
    if (firstUnderscore == String::npos)
    {
        return resolution::err("Underlying option ticker is in an incorrect format.");
    }

    String equitySymbol = optionString.substr(0, firstUnderscore);

    for (size_t i = 0; i < static_cast<size_t>(Equity::COUNT); ++i)
    {
        if (EQ_STR[i] == equitySymbol)
        {
            return static_cast<Equity>(i);
        }
    }

    return resolution::err(
        std::format("Extracted ticker: {} not found in list of equities.", equitySymbol));
}

OptionOrder::OptionOrder(int uid, Option optionTicker, Equity underlyingEquity, double price,
                         int qnty, MarketSide marketSide, TimePoint timeOrderPlaced, double strike,
                         OptionType optionType, double expiry)
    : Order(uid, optionTicker, price, qnty, marketSide, timeOrderPlaced),
      d_underlyingEquity(underlyingEquity),
      d_strike(strike),
      d_optionType(optionType),
      d_expiry(expiry)
{
}

Resolution<std::shared_ptr<OptionOrder>> OptionOrder::create(
    int uid, Option optionTicker, double price, int qnty, MarketSide marketSide,
    TimePoint timeOrderPlaced, double strike, OptionType optionType, double expiry)
{
    TimePoint d_timeOrderPlaced = timeNow();

    auto isOrderValid = validateOrderAttributes(price, qnty, timeOrderPlaced);
    if (!isOrderValid)
    {
        return resolution::err(isOrderValid.error());
    }

    auto underlyingEquity = extractUnderlyingEquity(optionTicker);
    if (!underlyingEquity)
    {
        return resolution::err(underlyingEquity.error());
    }

    auto optionOrder = std::shared_ptr<OptionOrder>(
        new (std::nothrow) OptionOrder{uid, optionTicker, *underlyingEquity, price, qnty,
                                       marketSide, timeOrderPlaced, strike, optionType, expiry});

    return optionOrder;
}

Resolution<std::shared_ptr<OptionOrder>> OptionOrder::createWithPricer(
    std::shared_ptr<pricing::Pricer> pricer, int uid, Option optionTicker)
{
    auto optionData = pricer->computeOptionData(optionTicker);
    const double theoreticalPrice = pricer->computeBlackScholes(optionData);

    // calculate actual market price, theoretical price as input
    const double marketPrice =
        pricer->calculateMarketPrice(optionData, theoreticalPrice, optionData.marketSide());

    // qnty is calculated from price, so compute last
    optionData.qnty(pricer->calculateQnty(optionTicker, optionData.marketSide(), marketPrice));

    const auto opt = OptionOrder::create(uid, optionTicker, marketPrice, optionData.qnty(),
                                         optionData.marketSide(), timeNow(), optionData.strike(),
                                         optionData.optionType(), optionData.expiry());
    if (!opt)
    {
        return resolution::err(opt.error());
    }

    auto greeks = pricer->computeGreeks(**opt);
    (*opt)->setGreeks(greeks);

    return *opt;
}

Resolution<std::shared_ptr<OptionOrder>> OptionOrder::createWithRandomValues(Config cfg, int uid,
                                                                             Option optionTicker)
{
    auto data = Random::generateOptionData(cfg);
    if (!data)
    {
        return resolution::err(data.error());
    }

    double optionPrice = Random::getRandomOptionPrice(cfg);

    auto opt =
        OptionOrder::create(uid, optionTicker, optionPrice, (*data).qnty(), (*data).marketSide(),
                            timeNow(), (*data).strike(), (*data).optionType(), (*data).expiry());
    if (!opt)
    {
        return resolution::err(opt.error());
    }

    auto greeks = Random::generateGreeks(*data);
    (*opt)->setGreeks(greeks);

    return *opt;
}

void OptionOrder::setGreeks(pricing::Greeks& greeks)
{
    this->delta(greeks.delta());
    this->gamma(greeks.gamma());
    this->theta(greeks.theta());
    this->vega(greeks.vega());
}

// getters

Equity OptionOrder::underlyingEquity() const { return d_underlyingEquity; }
double OptionOrder::strike() const { return d_strike; }
OptionType OptionOrder::optionType() const { return d_optionType; }
double OptionOrder::expiry() const { return d_expiry; }
double OptionOrder::delta() const { return d_delta; }
double OptionOrder::gamma() const { return d_gamma; }
double OptionOrder::theta() const { return d_theta; }
double OptionOrder::vega() const { return d_vega; }

// setters

void OptionOrder::underlyingEquity(Equity underlyingEquity)
{
    d_underlyingEquity = underlyingEquity;
}
void OptionOrder::strike(double strike) { d_strike = strike; }
void OptionOrder::optionType(OptionType optionType) { d_optionType = optionType; }
void OptionOrder::expiry(double expiry) { d_expiry = expiry; }
void OptionOrder::delta(double delta) { d_delta = delta; }
void OptionOrder::gamma(double gamma) { d_gamma = gamma; }
void OptionOrder::theta(double theta) { d_theta = theta; }
void OptionOrder::vega(double vega) { d_vega = vega; }

}  // namespace solstice
