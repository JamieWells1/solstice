#ifndef PRICING_DATA_H
#define PRICING_DATA_H

#include <asset_class.h>
#include <market_side.h>
#include <option_type.h>
#include <types.h>

namespace solstice::pricing
{

struct PricerDepOrderData
{
   public:
    PricerDepOrderData(Underlying underlying, MarketSide d_marketSide, double d_price, int d_qnty);

    Underlying underlying() const;
    MarketSide marketSide() const;
    double price() const;
    int qnty() const;

    void underlying(Underlying underlying);
    void marketSide(MarketSide marketSide);
    void price(double price);
    void qnty(int qnty);

   private:
    Underlying d_underlying;
    MarketSide d_marketSide;
    double d_price;
    double d_qnty;
};

struct PricerDepOptionData : public PricerDepOrderData
{
   public:
    PricerDepOptionData(Option optionTicker, Equity underlyingEquity, MarketSide d_marketSide,
                        double d_price, int d_qnty, double strike, OptionType optionType,
                        double expiry);

    Option optionTicker() const;
    Equity underlyingEquity() const;
    double strike() const;
    OptionType optionType() const;
    double expiry() const;

    void optionTicker(Option optionTicker);
    void underlyingEquity(Equity underlyingEquity);
    void strike(double strike);
    void optionType(OptionType optionType);
    void expiry(double expiry);

    friend class Pricer;

   private:
    // empty constructor for private calls from pricer
    PricerDepOptionData();

    Option d_optionTicker;
    Equity d_underlyingEquity;
    double d_strike;
    OptionType d_optionType;
    double d_expiry;
};

}  // namespace solstice::pricing

#endif  // PRICING_DATA_H
