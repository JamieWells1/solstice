#include <pricing_data.h>
#include <types.h>
#include <market_side.h>
#include <option_type.h>

namespace solstice::pricing
{

// PricerDepOrderData

PricerDepOrderData::PricerDepOrderData(Underlying underlying, MarketSide marketSide, double price, int qnty)
    : d_underlying(underlying), d_marketSide(marketSide), d_price(price), d_qnty(qnty)
{
}

MarketSide PricerDepOrderData::marketSide() const { return d_marketSide; }

double PricerDepOrderData::price() const { return d_price; }

int PricerDepOrderData::qnty() const { return d_qnty; }

// PricerDepOptionData

PricerDepOptionData::PricerDepOptionData(Underlying underlying, MarketSide marketSide, double price, int qnty,
                                         double strike, OptionType optionType, String expiry)
    : PricerDepOrderData(underlying, marketSide, price, qnty),
      d_strike(strike),
      d_optionType(optionType),
      d_expiry(expiry)
{
}

double PricerDepOptionData::strike() const { return d_strike; }

OptionType PricerDepOptionData::optionType() const { return d_optionType; }

String PricerDepOptionData::expiry() const { return d_expiry; }

}  // namespace solstice::pricing
