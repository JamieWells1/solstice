#include <market_side.h>
#include <option_type.h>
#include <pricing_data.h>
#include <types.h>

namespace solstice::pricing
{

// PricerDepOrderData

PricerDepOrderData::PricerDepOrderData(Underlying underlying, MarketSide marketSide, double price,
                                       int qnty)
    : d_underlying(underlying), d_marketSide(marketSide), d_price(price), d_qnty(qnty)
{
}

Underlying PricerDepOrderData::underlying() const { return d_underlying; }

MarketSide PricerDepOrderData::marketSide() const { return d_marketSide; }

double PricerDepOrderData::price() const { return d_price; }

int PricerDepOrderData::qnty() const { return d_qnty; }

void PricerDepOrderData::underlying(Underlying underlying) { d_underlying = underlying; }

void PricerDepOrderData::marketSide(MarketSide marketSide) { d_marketSide = marketSide; }

void PricerDepOrderData::price(double price) { d_price = price; }

void PricerDepOrderData::qnty(int qnty) { d_qnty = qnty; }

// PricerDepOptionData

// construct with default values that have to be modified after construction
PricerDepOptionData::PricerDepOptionData()
    : PricerDepOrderData(static_cast<Option>(0), static_cast<MarketSide>(0), -1, -1)
{
}

PricerDepOptionData::PricerDepOptionData(Option optionTicker, Equity underlyingEquity,
                                         MarketSide marketSide, double price, int qnty,
                                         double strike, OptionType optionType, double expiry)
    : PricerDepOrderData(optionTicker, marketSide, price, qnty),
      d_optionTicker(optionTicker),
      d_underlyingEquity(underlyingEquity),
      d_strike(strike),
      d_optionType(optionType),
      d_expiry(expiry)
{
}

Option PricerDepOptionData::optionTicker() const { return d_optionTicker; }

Equity PricerDepOptionData::underlyingEquity() const { return d_underlyingEquity; }

double PricerDepOptionData::strike() const { return d_strike; }

OptionType PricerDepOptionData::optionType() const { return d_optionType; }

double PricerDepOptionData::expiry() const { return d_expiry; }

void PricerDepOptionData::optionTicker(Option optionTicker) { d_optionTicker = optionTicker; }

void PricerDepOptionData::underlyingEquity(Equity underlyingEquity)
{
    d_underlyingEquity = underlyingEquity;
}

void PricerDepOptionData::strike(double strike) { d_strike = strike; }

void PricerDepOptionData::optionType(OptionType optionType) { d_optionType = optionType; }

void PricerDepOptionData::expiry(double expiry) { d_expiry = expiry; }

}  // namespace solstice::pricing
