#include <asset_class.h>
#include <get_random.h>
#include <market_side.h>
#include <order.h>
#include <pricer.h>
#include <types.h>

#include <format>
#include <memory>
#include <ostream>

namespace solstice
{

Order::Order(int uid, Underlying underlying, double price, int qnty, MarketSide marketSide,
             TimePoint timeOrderPlaced)
    : d_uid(uid),
      d_underlying(underlying),
      d_price(price),
      d_qnty(qnty),
      d_marketSide(marketSide),
      d_timeOrderPlaced(timeOrderPlaced)
{
    d_matched = false;
    d_outstandingQnty = qnty;
    d_assetClass = underlying.index();
}

std::expected<std::shared_ptr<Order>, String> Order::create(int uid, Underlying underlying,
                                                            double price, int qnty,
                                                            MarketSide marketSide)
{
    TimePoint timeOrderPlaced = timeNow();

    auto isOrderValid = validateOrderAttributes(price, qnty, timeOrderPlaced);
    if (!isOrderValid)
    {
        return std::unexpected(isOrderValid.error());
    }

    auto order = std::shared_ptr<Order>(
        new (std::nothrow) Order{uid, underlying, price, qnty, marketSide, timeOrderPlaced});

    return order;
}

std::expected<std::shared_ptr<Order>, String> Order::createWithPricer(
    std::shared_ptr<pricing::Pricer> pricer, int uid, Underlying underlying)
{
    auto data = pricer->computeOrderData(underlying);
    if (!data)
    {
        return std::unexpected(data.error());
    }

    return Order::create(uid, underlying, data->price(), data->qnty(), data->marketSide());
}

std::expected<std::shared_ptr<Order>, String> Order::createWithRandomValues(Config cfg, int uid,
                                                                            Underlying underlying)
{
    auto data = Random::generateOrderData(cfg);
    return Order::create(uid, underlying, data->price(), data->qnty(), data->marketSide());
}

// getters

int Order::uid() const { return d_uid; }

Underlying Order::underlying() const { return d_underlying; }

AssetClass Order::assetClass() const { return static_cast<AssetClass>(d_underlying.index()); }

double Order::price() const
{
    // d_price becomes irrelevant once an order is matched
    if (matched())
    {
        return d_matchedPrice;
    }
    return d_price;
}

int Order::qnty() const { return d_qnty; }

int Order::outstandingQnty() const { return d_outstandingQnty; }

int Order::outstandingQnty(int newOutstandingQnty)
{
    d_outstandingQnty = newOutstandingQnty;
    return newOutstandingQnty;
};

MarketSide Order::marketSide() const { return d_marketSide; }

String Order::marketSideString() const
{
    return d_marketSide == solstice::MarketSide::Bid ? "Bid" : "ask";
}

TimePoint Order::timeOrderPlaced() const { return d_timeOrderPlaced; }

bool Order::matched() const { return d_matched; }

double Order::matchedPrice() const { return d_matchedPrice; }

// setters

void Order::price(double newPrice)
{
    d_price = newPrice;
}

void Order::matched(bool isFulfilled) { d_matched = isFulfilled; }

void Order::matchedPrice(double matchedPrice) { d_matchedPrice = matchedPrice; }

std::expected<TimePoint, String> Order::timeOrderFulfilled() const
{
    // Cannot return time of fulfillment if fulfillment hasn't yet occured
    if (!d_matched)
    {
        return std::unexpected("Order has not been fulfilled yet\n");
    }
    return d_timeOrderFulfilled;
}

std::expected<void, String> Order::validatePrice(const double price)
{
    if (price < 0)
    {
        return std::unexpected(std::format("Invalid price: {}", price, "\n"));
    }

    return {};
}

std::expected<void, String> Order::validateQnty(const int qnty)
{
    if (qnty < 0)
    {
        return std::unexpected(std::format("Invalid quantity: {}", qnty, "\n"));
    }

    return {};
}

std::expected<void, String> Order::validateOrderAttributes(double price, int qnty,
                                                           TimePoint& timeOrderPlaced)
{
    auto validPrice = Order::validatePrice(price);
    auto validQnty = Order::validateQnty(qnty);

    if (!validPrice)
    {
        return std::unexpected(validPrice.error());
    }

    if (!validQnty)
    {
        return std::unexpected(validQnty.error());
    }

    return {};
}

std::ostream& operator<<(std::ostream& os, const Order& order)
{
    os << "Order UID: " << order.uid() << " | Ticker: " << to_string(order.underlying())
       << " | Price: " << order.price() << " | Quantity: " << order.qnty()
       << " | Is bid: " << std::boolalpha << (order.marketSide() == MarketSide::Bid);

    return os;
}
}  // namespace solstice
