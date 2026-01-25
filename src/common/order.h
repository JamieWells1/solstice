#ifndef ORDER_H
#define ORDER_H

#include <asset_class.h>
#include <config.h>
#include <market_side.h>
#include <time_point.h>
#include <types.h>

#include <ctime>
#include <memory>
#include <resolution.hpp>
#include <variant>

namespace solstice::pricing
{
class Pricer;
}

namespace solstice
{

class Order
{
   public:
    virtual ~Order() = default;

    static Resolution<std::shared_ptr<Order>> create(int uid, Underlying underlying, double price,
                                                     int qnty, MarketSide marketSide);

    static Resolution<std::shared_ptr<Order>> createWithPricer(
        std::shared_ptr<pricing::Pricer> pricer, int uid, Underlying underlying);

    static Resolution<std::shared_ptr<Order>> createWithRandomValues(Config d_config, int uid,
                                                                     Underlying underlying);

    int uid() const;
    Underlying underlying() const;
    AssetClass assetClass() const;
    double price() const;
    int qnty() const;
    int outstandingQnty() const;
    MarketSide marketSide() const;
    String marketSideString() const;
    TimePoint timeOrderPlaced() const;
    Resolution<TimePoint> timeOrderFulfilled() const;
    int outstandingQnty(int newQnty);
    bool matched() const;
    double matchedPrice() const;

    void price(double newPrice);
    void matched(bool isFulfilled);
    void matchedPrice(double matchedPrice);

   protected:
    Order(int uid, Underlying underlying, double price, int qnty, MarketSide marketSide,
          TimePoint timeOrderPlaced);

    static Resolution<std::monostate> validatePrice(const double price);
    static Resolution<std::monostate> validateQnty(const int qnty);
    static Resolution<std::monostate> validateOrderAttributes(double price, int qnty,
                                                              TimePoint& timeOrderPlaced);

    int d_uid;
    Underlying d_underlying;
    size_t d_assetClass;
    double d_price;
    int d_qnty;
    int d_outstandingQnty;
    MarketSide d_marketSide;
    TimePoint d_timeOrderPlaced;
    TimePoint d_timeOrderFulfilled;
    bool d_matched;
    double d_matchedPrice;
};

std::ostream& operator<<(std::ostream& os, const Order& order);

}  // namespace solstice

#endif
