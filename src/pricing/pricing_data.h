#ifndef PRICING_DATA_H
#define PRICING_DATA_H

#include <types.h>
#include <market_side.h>
#include <option_type.h>

namespace solstice::pricing
{

struct PricerDepOrderData
{
   public:
    PricerDepOrderData(MarketSide d_marketSide, double d_price, int d_qnty);

    MarketSide marketSide() const;
    double price() const;
    int qnty() const;

   private:
    MarketSide d_marketSide;
    double d_price;
    double d_qnty;
};

struct PricerDepOptionData : public PricerDepOrderData
{
   public:
    PricerDepOptionData(MarketSide d_marketSide, double d_price, int d_qnty, double strike,
                        OptionType optionType, String expiry);

    double strike() const;
    OptionType optionType() const;
    String expiry() const;

   private:
    double d_strike;
    OptionType d_optionType;
    String d_expiry;
};

}  // namespace solstice::pricing

#endif  // PRICING_DATA_H
