#include <market_side.h>
#include <option_type.h>
#include <pricing_data.h>
#include <types.h>

namespace solstice::pricing
{

// PricerDepOrderData

PricerDepOrderData::PricerDepOrderData(Underlying underlying,
                                       MarketSide marketSide, double price, int qnty)
    : d_underlying(underlying),
      d_marketSide(marketSide),
      d_price(price),
      d_qnty(qnty)
{
}

Underlying PricerDepOrderData::underlying() const { return d_underlying; }

MarketSide PricerDepOrderData::marketSide() const { return d_marketSide; }

double PricerDepOrderData::price() const { return d_price; }

int PricerDepOrderData::qnty() const { return d_qnty; }

// PricerDepOptionData

PricerDepOptionData::PricerDepOptionData(Underlying optionTicker, Underlying underlyingAsset, MarketSide marketSide,
                                         double price, int qnty, double strike,
                                         OptionType optionType, String expiry)
    : PricerDepOrderData(optionTicker, marketSide, price, qnty),
      d_strike(strike),
      d_optionType(optionType),
      d_expiry(expiry)
{
}

Underlying PricerDepOptionData::optionTicker() const { return d_optionTicker; }

Underlying PricerDepOptionData::underlyingAsset() const { return d_underlyingAsset; }

double PricerDepOptionData::strike() const { return d_strike; }

OptionType PricerDepOptionData::optionType() const { return d_optionType; }

String PricerDepOptionData::expiry() const { return d_expiry; }

}  // namespace solstice::pricing
