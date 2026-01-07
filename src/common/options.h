#ifndef OPTIONS_H
#define OPTIONS_H

#include <greeks.h>
#include <option_type.h>
#include <order.h>
#include <types.h>

namespace solstice
{

std::expected<Underlying, String> extractUnderlyingEquity(Underlying underlying);

class OptionOrder : public Order
{
   public:
    static std::expected<std::shared_ptr<OptionOrder>, String> create(
        int uid, Underlying underlying, double price, int qnty, MarketSide marketSide,
        TimePoint timeOrderPlaced, double strike, OptionType optionType, String expiry);

    static std::expected<std::shared_ptr<OptionOrder>, String> createWithPricer(
        std::shared_ptr<pricing::Pricer> pricer, int uid, Underlying underlying);

    static std::expected<std::shared_ptr<OptionOrder>, String> createWithRandomValues(
        Config d_config, int uid, Underlying underlying);

    Underlying underlyingAsset();
    double strike();
    OptionType optionType();
    String expiry();
    double delta();
    double gamma();
    double theta();
    double vega();

    void underlyingAsset(Underlying underlyingAsset);
    void strike(double strike);
    void optionType(OptionType optionType);
    void expiry(String expiry);
    void delta(double delta);
    void gamma(double gamma);
    void theta(double theta);
    void vega(double vega);

   private:
    OptionOrder(int uid, Underlying underlying, Underlying underlyingAsset, double price, int qnty, MarketSide marketSide,
                TimePoint timeOrderPlaced, double strike, OptionType optionType, String expiry);

    void setGreeks(pricing::Greeks& greeks);

    Underlying d_underlyingAsset;
    double d_strike;
    OptionType d_optionType;
    String d_expiry;
    double d_delta;
    double d_gamma;
    double d_theta;
    double d_vega;
};

}  // namespace solstice

#endif  // OPTIONS_H
