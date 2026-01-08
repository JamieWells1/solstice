#include <asset_class.h>
#include <greeks.h>
#include <options.h>
#include <order.h>
#include <pricer.h>
#include <time_point.h>
#include <types.h>

#include <format>

namespace solstice
{

std::expected<Equity, String> extractUnderlyingEquity(Equity optionTicker)
{
    String optionString = to_string(optionTicker);

    size_t firstUnderscore = optionString.find('_');
    if (firstUnderscore == String::npos)
    {
        return std::unexpected("Underlying option ticker is in an incorrect format.");
    }

    String equitySymbol = optionString.substr(0, firstUnderscore);

    for (size_t i = 0; i < static_cast<size_t>(Equity::COUNT); ++i)
    {
        if (EQ_STR[i] == equitySymbol)
        {
            return static_cast<Equity>(i);
        }
    }

    return std::unexpected(
        std::format("Extracted ticker: {} not found in list of equities.", equitySymbol));
}

OptionOrder::OptionOrder(int uid, Option optionTicker, Equity underlyingEquity,
                         double price, int qnty, MarketSide marketSide, TimePoint timeOrderPlaced,
                         double strike, OptionType optionType, String expiry)
    : Order(uid, optionTicker, price, qnty, marketSide, timeOrderPlaced),
      d_strike(strike),
      d_optionType(optionType)
{
}

std::expected<std::shared_ptr<OptionOrder>, String> OptionOrder::create(
    int uid, Option optionTicker, double price, int qnty, MarketSide marketSide,
    TimePoint timeOrderPlaced, double strike, OptionType optionType, String expiry)
{
    TimePoint d_timeOrderPlaced = timeNow();

    auto isOrderValid = validateOrderAttributes(price, qnty, timeOrderPlaced);
    if (!isOrderValid)
    {
        return std::unexpected(isOrderValid.error());
    }

    auto underlyingEquity = extractUnderlyingEquity(optionTicker);
    if (!underlyingEquity)
    {
        return std::unexpected(underlyingEquity.error());
    }

    auto optionOrder = std::shared_ptr<OptionOrder>(
        new (std::nothrow) OptionOrder{uid, optionTicker, *underlyingEquity, price, qnty,
                                       marketSide, timeOrderPlaced, strike, optionType, expiry});

    return optionOrder;
}

std::expected<std::shared_ptr<OptionOrder>, String> OptionOrder::createWithPricer(
    std::shared_ptr<pricing::Pricer> pricer, int uid, Option optionTicker)
{
    auto optionData = pricer->computeOptionData(optionTicker);
    double optionPrice = pricer->computeBlackScholes(optionData);

    auto opt = OptionOrder::create(uid, optionTicker, optionPrice, optionData.qnty(),
                                   optionData.marketSide(), timeNow(), optionData.strike(),
                                   optionData.optionType(), optionData.expiry());
    if (!opt)
    {
        return std::unexpected(opt.error());
    }

    auto greeks = pricer->computeGreeks(**opt);
    (*opt)->setGreeks(greeks);

    return *opt;
}

std::expected<std::shared_ptr<OptionOrder>, String> OptionOrder::createWithRandomValues(
    Config cfg, int uid, Option optionTicker)
{
    auto data = Random::generateOptionData(cfg);
    if (!data)
    {
        return std::unexpected(data.error());
    }

    double optionPrice = Random::getRandomOptionPrice(cfg);

    auto opt = OptionOrder::create(uid, optionTicker, optionPrice, data->qnty(), data->marketSide(),
                                   timeNow(), data->strike(), data->optionType(), data->expiry());
    if (!opt)
    {
        return std::unexpected(opt.error());
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
String OptionOrder::expiry() const { return d_expiry; }
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
void OptionOrder::expiry(String expiry) { d_expiry = expiry; }
void OptionOrder::delta(double delta) { d_delta = delta; }
void OptionOrder::gamma(double gamma) { d_gamma = gamma; }
void OptionOrder::theta(double theta) { d_theta = theta; }
void OptionOrder::vega(double vega) { d_vega = vega; }

}  // namespace solstice
