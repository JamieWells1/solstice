#ifndef OPTIONS_H
#define OPTIONS_H

#include <greeks.h>
#include <option_type.h>
#include <order.h>
#include <types.h>

namespace solstice
{

std::expected<Equity, String> extractUnderlyingEquity(Option optionTicker);

class OptionOrder : public Order
{
   public:
    static std::expected<std::shared_ptr<OptionOrder>, String> create(
        int uid, Option optionTicker, double price, int qnty, MarketSide marketSide,
        TimePoint timeOrderPlaced, double strike, OptionType optionType, double expiry);

    static std::expected<std::shared_ptr<OptionOrder>, String> createWithPricer(
        std::shared_ptr<pricing::Pricer> pricer, int uid, Option optionTicker);

    static std::expected<std::shared_ptr<OptionOrder>, String> createWithRandomValues(
        Config d_config, int uid, Option optionTicker);

    Equity underlyingEquity() const;
    double strike() const;
    OptionType optionType() const;
    double expiry() const;
    double delta() const;
    double gamma() const;
    double theta() const;
    double vega() const;

    void underlyingEquity(Equity underlyingEquity);
    void strike(double strike);
    void optionType(OptionType optionType);
    void expiry(double expiry);
    void delta(double delta);
    void gamma(double gamma);
    void theta(double theta);
    void vega(double vega);

   private:
    OptionOrder(int uid, Option optionTicker, Equity underlyingEquity, double price, int qnty,
                MarketSide marketSide, TimePoint timeOrderPlaced, double strike,
                OptionType optionType, double expiry);

    void setGreeks(pricing::Greeks& greeks);

    Equity d_underlyingEquity;
    double d_strike;
    OptionType d_optionType;
    double d_expiry;
    double d_delta;
    double d_gamma;
    double d_theta;
    double d_vega;
};

}  // namespace solstice

#endif  // OPTIONS_H
