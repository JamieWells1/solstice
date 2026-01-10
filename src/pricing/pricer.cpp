#include <asset_class.h>
#include <config.h>
#include <get_random.h>
#include <market_side.h>
#include <option_price_data.h>
#include <option_type.h>
#include <order_type.h>
#include <pricer.h>
#include <pricing_utils.h>
#include <time_point.h>
#include <types.h>

#include <cmath>
#include <numbers>
#include <unordered_map>

namespace solstice::pricing
{

using std::numbers::pi;

// risk-free rate for derivatives pricing
constexpr double r = 0.05;

constexpr double baseOrderValue = 10000;

const std::unordered_map<OrderType, double> probabilities = {
    {OrderType::CrossSpread, 0.3}, {OrderType::InsideSpread, 0.2}, {OrderType::AtSpread, 0.5}};

// equity pricing calc constants
constexpr double EQUITY_INITIAL_SPREAD_PCT = 0.002;             // 0.2% initial spread
constexpr double EQUITY_BASE_SPREAD_PCT = 0.002;                // Base spread width
constexpr double EQUITY_VOLATILITY_SPREAD_MULTIPLIER = 0.0015;  // Volatility impact on spread
constexpr double EQUITY_SPREAD_ADJUSTMENT_WEIGHT = 0.95;  // Current spread weight in adjustment
constexpr double EQUITY_TARGET_ADJUSTMENT_WEIGHT = 0.05;  // Target spread weight in adjustment
constexpr double EQUITY_MIN_EXEC_FOR_SPREAD_CALC = 10;  // Min executions before spread calculation
constexpr double EQUITY_TRANSIENT_DRIFT_PCT = 0.025;    // ±2.5% transient price drift
constexpr double EQUITY_DECAY_FACTOR = 0.94;

// future pricing calc constants
constexpr double FUTURE_INITIAL_SPREAD_PCT = 0.01;            // 1% initial spread
constexpr double FUTURE_BASE_SPREAD_PCT = 0.005;              // 0.5% base spread
constexpr double FUTURE_VOLATILITY_SPREAD_MULTIPLIER = 0.01;  // Volatility impact on spread

// price calc constants
constexpr double INSIDE_SPREAD_SHIFT_FACTOR = 0.5;  // 50% of half-spread for shift
constexpr double INSIDE_SPREAD_RANGE_FACTOR = 0.3;  // 30% of half-spread as range
constexpr double CROSS_SPREAD_OFFSET_FACTOR = 0.5;  // 50% of half-spread for offset

// quantity calc constants
constexpr double MIN_DEMAND_SCALE = 0.3;    // Minimum demand scale
constexpr double MAX_DEMAND_SCALE = 0.7;    // Additional demand scale based on demand factor
constexpr double MAX_VOL_ADJUSTMENT = 0.5;  // Maximum volatility adjustment
constexpr int MIN_QUANTITY_THRESHOLD = 10;  // Minimum quantity threshold
constexpr int MIN_QUANTITY = 1;             // Minimum order quantity

namespace
{

double timeToExpiry(Future fut)
{
    String name = to_string(fut);
    CurrentDate dateNow = currentDate();

    int expiryMonth = monthToInt(name.substr(name.length() - 5, 3));
    int expiryYear = std::stoi(name.substr(name.length() - 2, 2));

    double monthsToExpiry;

    // NOTE: expiry year is ignored to avoid expiration in the future if securities are not updated
    if (expiryMonth == dateNow.month)
    {
        monthsToExpiry = 1.0;
    }
    else
    {
        monthsToExpiry = std::abs(expiryMonth - dateNow.month);
    }
    // convert to years
    return monthsToExpiry / 12;
}

double timeToExpiry(Option opt)
{
    String name = to_string(opt);
    CurrentDate dateNow = currentDate();

    int expiryMonth = monthToInt(name.substr(name.length() - 7, 3));
    int expiryYear = std::stoi(name.substr(name.length() - 4, 2));

    double monthsToExpiry;

    // NOTE: expiry year is ignored to avoid expiration in the future if securities are not updated
    if (expiryMonth == dateNow.month)
    {
        monthsToExpiry = 1.0;
    }
    else
    {
        monthsToExpiry = std::abs(expiryMonth - dateNow.month);
    }
    // convert to years
    return monthsToExpiry / 12;
}

}  // namespace

// Pricer

Pricer::Pricer(std::shared_ptr<matching::OrderBook> orderBook) : d_orderBook(orderBook)
{
    d_seedPrice = generateSeedPrice();
}

std::shared_ptr<matching::OrderBook> Pricer::orderBook() { return d_orderBook; }

// ===================================================================
// PRE-PROCESSING
// PRE-PROCESSING
// ===================================================================

double Pricer::generateSeedPrice()
{
    Config cfg = Config::instance().value();
    return Random::getRandomDouble(cfg.minPrice(), cfg.maxPrice());
}

MarketSide Pricer::calculateMarketSide(Equity eq)
{
    EquityPriceData data = orderBook()->getPriceData(eq);
    double p = data.demandFactor() * data.demandFactor();

    return calculateMarketSideImpl(p);
}

MarketSide Pricer::calculateMarketSide(Future fut)
{
    FuturePriceData data = orderBook()->getPriceData(fut);
    double p = data.demandFactor() * data.demandFactor();

    return calculateMarketSideImpl(p);
}

MarketSide Pricer::calculateMarketSide(Option opt)
{
    OptionPriceData data = orderBook()->getPriceData(opt);
    double p = data.demandFactor() * data.demandFactor();

    return calculateMarketSideImpl(p);
}

MarketSide Pricer::calculateMarketSideImpl(double probability)
{
    double random = Random::getRandomDouble(-1, 1);

    MarketSide mktSide;

    // higher probability of bid/ask if higher demand factor
    bool isBid = random < probability && random > 0;
    bool isAsk = random > probability && random < 0;

    if (isBid)
    {
        return MarketSide::Bid;
    }

    if (isAsk)
    {
        return MarketSide::Ask;
    }

    // get random if random number doesn't fall within threshold
    return Random::getRandomMarketSide();
}

OrderType Pricer::getOrderType()
{
    OrderType type;
    double random = Random::getRandomDouble(0, 1);

    bool inCrossSpreadBand = random < probabilities.at(OrderType::CrossSpread);

    bool inInsideSpreadBand = random >= probabilities.at(OrderType::CrossSpread) &&
                              random < (probabilities.at(OrderType::CrossSpread) +
                                        probabilities.at(OrderType::InsideSpread));

    bool inAtSpreadBand = random >= (probabilities.at(OrderType::CrossSpread) +
                                     probabilities.at(OrderType::InsideSpread));

    if (inCrossSpreadBand)
    {
        type = OrderType::CrossSpread;
    }
    else if (inInsideSpreadBand)
    {
        type = OrderType::InsideSpread;
    }
    else if (inAtSpreadBand)
    {
        type = OrderType::AtSpread;
    }

    return type;
}

double Pricer::calculateMarketPriceImpl(MarketSide mktSide, double lowestAsk, double highestBid,
                                        double demandFactor)
{
    double price = 0.0;
    OrderType type = getOrderType();

    double spread = lowestAsk - highestBid;
    double midSpread = (lowestAsk + highestBid) / 2;
    double halfSpread = (midSpread - highestBid);

    // order follows bullish momentum
    if (mktSide == MarketSide::Bid)
    {
        switch (type)
        {
            case solstice::OrderType::InsideSpread:
            {
                if (spread > 0)
                {
                    double shift = halfSpread * demandFactor * INSIDE_SPREAD_SHIFT_FACTOR;
                    double targetPrice = midSpread + shift;
                    double priceRange = halfSpread * INSIDE_SPREAD_RANGE_FACTOR;

                    double priceLowerBound = std::max(highestBid, targetPrice - priceRange);
                    double priceUpperBound = std::min(lowestAsk, targetPrice + priceRange);

                    price = Random::getRandomDouble(priceLowerBound, priceUpperBound);
                }
                else
                {
                    price = highestBid;
                }
                break;
            }

            case (OrderType::CrossSpread):
            {
                if (spread > 0)
                {
                    double offset =
                        halfSpread * std::abs(demandFactor) * CROSS_SPREAD_OFFSET_FACTOR;

                    double priceLowerBound = lowestAsk;
                    double priceUpperBound = lowestAsk + offset;

                    price = Random::getRandomDouble(priceLowerBound, priceUpperBound);
                }
                else
                {
                    price = lowestAsk;
                }
                break;
            }

            case (OrderType::AtSpread):
            {
                price = highestBid;
                break;
            }
        }
    }

    // order follows bearish momentum
    if (mktSide == MarketSide::Ask)
    {
        switch (type)
        {
            case solstice::OrderType::InsideSpread:
            {
                if (spread > 0)
                {
                    double shift = halfSpread * demandFactor * INSIDE_SPREAD_SHIFT_FACTOR;
                    double targetPrice = midSpread + shift;
                    double priceRange = halfSpread * INSIDE_SPREAD_RANGE_FACTOR;

                    double priceLowerBound = std::max(highestBid, targetPrice - priceRange);
                    double priceUpperBound = std::min(lowestAsk, targetPrice + priceRange);

                    price = Random::getRandomDouble(priceLowerBound, priceUpperBound);
                }
                else
                {
                    price = lowestAsk;
                }
                break;
            }

            case (OrderType::CrossSpread):
            {
                if (spread > 0)
                {
                    double offset =
                        halfSpread * std::abs(demandFactor) * CROSS_SPREAD_OFFSET_FACTOR;

                    double priceUpperBound = highestBid;
                    double priceLowerBound = std::max(1.0, highestBid - offset);

                    price = Random::getRandomDouble(priceLowerBound, priceUpperBound);
                }
                else
                {
                    price = highestBid;
                }
                break;
            }

            case (OrderType::AtSpread):
            {
                price = lowestAsk;
                break;
            }
        }
    }

    return std::max(1.0, price);
}

double Pricer::calculateCarryAdjustment(Future fut)
{
    FuturePriceData data = orderBook()->getPriceData(fut);

    double spot = data.lastPrice();
    double t = timeToExpiry(fut);

    // where r is risk-free rate
    return spot * std::exp(r * t) - spot;
}

double Pricer::calculateMarketPrice(Equity eq, MarketSide mktSide)
{
    EquityPriceData& data = orderBook()->getPriceData(eq);

    if (data.highestBid() == 0.0 && data.lowestAsk() == 0.0)
    {
        double initialPrice = data.lastPrice();
        double spreadWidth = initialPrice * EQUITY_INITIAL_SPREAD_PCT;

        data.highestBid(initialPrice - spreadWidth / 2);
        data.lowestAsk(initialPrice + spreadWidth / 2);
    }
    else if (data.executions() >= EQUITY_MIN_EXEC_FOR_SPREAD_CALC)
    {
        double basePrice = data.movingAverage();
        double sigma = data.standardDeviation(data);

        double spreadWidth =
            basePrice * (EQUITY_BASE_SPREAD_PCT + sigma * EQUITY_VOLATILITY_SPREAD_MULTIPLIER);

        double targetBid = basePrice - spreadWidth / 2;
        double targetAsk = basePrice + spreadWidth / 2;

        data.highestBid(data.highestBid() * EQUITY_SPREAD_ADJUSTMENT_WEIGHT +
                        targetBid * EQUITY_TARGET_ADJUSTMENT_WEIGHT);
        data.lowestAsk(data.lowestAsk() * EQUITY_SPREAD_ADJUSTMENT_WEIGHT +
                       targetAsk * EQUITY_TARGET_ADJUSTMENT_WEIGHT);
    }

    double bidDrift =
        Random::getRandomDouble(-EQUITY_TRANSIENT_DRIFT_PCT, EQUITY_TRANSIENT_DRIFT_PCT);
    double askDrift =
        Random::getRandomDouble(-EQUITY_TRANSIENT_DRIFT_PCT, EQUITY_TRANSIENT_DRIFT_PCT);

    double adjustedBid = data.highestBid() * (1.0 + bidDrift);
    double adjustedAsk = data.lowestAsk() * (1.0 + askDrift);

    return calculateMarketPriceImpl(mktSide, adjustedAsk, adjustedBid, data.demandFactor());
}

double Pricer::calculateMarketPrice(Future fut, MarketSide mktSide)
{
    FuturePriceData& data = orderBook()->getPriceData(fut);
    double basePrice = data.lastPrice();

    if (data.executions() > 0)
    {
        basePrice = data.movingAverage();
    }

    double spreadWidth;
    if (data.executions() > 1)
    {
        double sigma = data.standardDeviation(data);
        spreadWidth =
            basePrice * (FUTURE_BASE_SPREAD_PCT + sigma * FUTURE_VOLATILITY_SPREAD_MULTIPLIER);
    }
    else
    {
        spreadWidth = basePrice * FUTURE_INITIAL_SPREAD_PCT;
    }

    data.highestBid(basePrice - spreadWidth / 2);
    data.lowestAsk(basePrice + spreadWidth / 2);

    double costOfCarry = calculateCarryAdjustment(fut);
    double adjustedBid = data.highestBid() + costOfCarry;
    double adjustedAsk = data.lowestAsk() + costOfCarry;

    return calculateMarketPriceImpl(mktSide, adjustedAsk, adjustedBid, data.demandFactor());
}

double Pricer::calculateMarketPrice(Option opt, MarketSide mktSide)
{
    // TODO
}

int Pricer::calculateQnty(Equity eq, MarketSide mktSide, double price)
{
    EquityPriceData data = orderBook()->getPriceData(eq);
    double n = data.executions();

    double demandScale = MIN_DEMAND_SCALE + (MAX_DEMAND_SCALE * std::abs(data.demandFactor()));

    double sigma = n > 1 ? data.standardDeviation(data) : 0;
    double volAdjustment = std::min(sigma, MAX_VOL_ADJUSTMENT);

    int maxQuantity = baseOrderValue * demandScale / (price * (1 + volAdjustment));
    if (maxQuantity < MIN_QUANTITY_THRESHOLD)
        return Random::getRandomInt(MIN_QUANTITY, MIN_QUANTITY_THRESHOLD);

    return Random::getRandomInt(MIN_QUANTITY, maxQuantity);
}

int Pricer::calculateQnty(Future fut, MarketSide mktSide, double price)
{
    FuturePriceData data = orderBook()->getPriceData(fut);
    double n = data.executions();
    double demandScale = MIN_DEMAND_SCALE + (MAX_DEMAND_SCALE * std::abs(data.demandFactor()));

    double sigma = n > 1 ? data.standardDeviation(data) : 0;
    double volAdjustment = std::min(sigma, MAX_VOL_ADJUSTMENT);

    int maxQuantity = baseOrderValue * demandScale / (price * (1 + volAdjustment));

    if (maxQuantity < MIN_QUANTITY_THRESHOLD)
        return Random::getRandomInt(MIN_QUANTITY, MIN_QUANTITY_THRESHOLD);

    return Random::getRandomInt(MIN_QUANTITY, maxQuantity);
}

int Pricer::calculateQnty(Option opt, MarketSide mktSide, double price)
{
    // TODO
}

double Pricer::calculateStrikeImpl(PricerDepOptionData& data)
{
    double strikeLowerBound;
    double strikeUpperBound;
    double strike;

    auto equityPriceData = orderBook()->getPriceData(data.underlyingEquity());
    double spot = equityPriceData.lastPrice();

    // integer to determine if option is OTM, ATM or ITM
    double moneyCall = Random::getRandomInt(1, 100);

    if (data.optionType() == OptionType::Call)
    {
        if (moneyCall <= 25)
        {
            // ITM, strike > spot
            strikeLowerBound = spot + (0.01 * spot);
            strikeUpperBound = spot + (0.15 * spot);
        }
        else if (moneyCall > 25 && moneyCall <= 95)
        {
            // OTM, strike < spot
            strikeLowerBound = spot - (0.01 * spot);
            strikeUpperBound = spot - (0.15 * spot);
        }
        else
        {
            // ATM, strike == spot
            strikeLowerBound = spot - (0.005 * spot);
            strikeUpperBound = spot + (0.005 * spot);
        }
    }
    else  // PUT
    {
        if (moneyCall <= 25)
        {
            // ITM, strike < spot
            strikeLowerBound = spot - (0.01 * spot);
            strikeUpperBound = spot - (0.15 * spot);
        }
        else if (moneyCall > 25 && moneyCall <= 95)
        {
            // OTM, strike > spot
            strikeLowerBound = spot + (0.01 * spot);
            strikeUpperBound = spot + (0.15 * spot);
        }
        else
        {
            // ATM, strike == spot
            strikeLowerBound = spot - (0.005 * spot);
            strikeUpperBound = spot + (0.005 * spot);
        }
    }

    // band increment, usually ~1% of spot price (e.g. $2.50 for $250 AAPL stock)
    double bandIncrement = 

    // TODO

    // ======================================== NOTES: ========================================
    // - Call OTM = strike > spot
    // - Put OTM = strike < spot
    // - ~70% OTM, ~25% ITM, ~5% ATM
    // - Implement band calculation for standard strike increments based on price
    // - Increments every 1% of stock price, (e.g. $2.50 for $250 AAPL stock),
    //   max range 10-15% of spot price
    // ========================================================================================

    return strike;
}

PricerDepOptionData Pricer::computeOptionData(Option opt)
{
    // use private empty constructor as we are not populating the parent qnty and price attributes
    PricerDepOptionData data;

    data.optionTicker(opt);
    data.underlyingEquity(*extractUnderlyingEquity(opt));
    data.strike(calculateStrikeImpl(data));
    data.marketSide(Random::getRandomMarketSide());
    data.optionType(Random::getRandomOptionType());
    data.expiry(timeToExpiry(opt));

    return data;
}

double Pricer::computeBlackScholes(PricerDepOptionData& optionData)
{
    auto underlyingEquity = orderBook()->getPriceData(optionData.underlyingEquity());

    double S = underlyingEquity.lastPrice();
    double K = optionData.strike();
    double sigma = underlyingEquity.volatility();
    // r is already defined as a constant at the top of the file

    double T = optionData.expiry();

    const double numeratorD1 = std::log(S / K) + (r + sigma * sigma / 2) * T;
    const double denominatorD1 = sigma * std::sqrt(T);
    const double d1 = numeratorD1 / denominatorD1;

    const double d2 = d1 - (sigma * std::sqrt(T));

    double price = -1;

    if (optionData.optionType() == OptionType::Call)
    {
        price = (S * N(d1)) - (K * std::exp(-r * T) * N(d2));
    }
    else  // Put
    {
        price = (K * std::exp(-r * T) * (1 - N(d2))) - (S * (1 - N(d1)));
    }

    return price;
}

Greeks Pricer::computeGreeks(OptionOrder& option)
{
    Option optionTicker = std::get<Option>(option.underlying());

    auto priceData = orderBook()->getPriceData(option.underlyingEquity());
    double S = priceData.lastPrice();
    double sigma = priceData.volatility();
    double K = option.strike();
    double T = timeToExpiry(optionTicker);
    // r is already defined as a constant at the top of the file

    double d1 = (std::log(S / K) + (r + sigma * sigma / 2.0) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);

    double n_d1 = (1.0 / std::sqrt(2.0 * pi)) * std::exp(-d1 * d1 / 2.0);

    double N_d1 = 0.5 * (1.0 + std::erf(d1 / std::sqrt(2.0)));
    double N_d2 = 0.5 * (1.0 + std::erf(d2 / std::sqrt(2.0)));

    double delta, theta;

    if (option.optionType() == OptionType::Call)
    {
        delta = N_d1;
        theta = -(S * n_d1 * sigma) / (2.0 * std::sqrt(T)) - r * K * std::exp(-r * T) * N_d2;
    }
    else  // Put
    {
        delta = N_d1 - 1.0;
        theta =
            -(S * n_d1 * sigma) / (2.0 * std::sqrt(T)) + r * K * std::exp(-r * T) * (1.0 - N_d2);
    }

    double gamma = n_d1 / (S * sigma * std::sqrt(T));
    double vega = S * std::sqrt(T) * n_d1;

    return Greeks(delta, gamma, theta, vega);
}

// ===================================================================
// POST-PROCESSING
// ===================================================================

void Pricer::update(matching::OrderPtr order)
{
    withPriceData(
        order->underlying(),
        [&order, this](auto& priceData)
        {
            bool isBid = order->marketSide() == MarketSide::Bid;
            bool isAsk = order->marketSide() == MarketSide::Ask;

            if (order->matched())
            {
                double matchedPrice = order->matchedPrice();

                if (isBid && (!priceData.highestBid() || priceData.highestBid() < matchedPrice))
                {
                    priceData.highestBid(matchedPrice);
                }

                if (isAsk && (!priceData.lowestAsk() || priceData.lowestAsk() > matchedPrice))
                {
                    priceData.lowestAsk(matchedPrice);
                }

                priceData.lastPrice(matchedPrice);
                priceData.updateVolatility(matchedPrice);

                if (priceData.executions() >= 10)
                {
                    priceData.pricesSum(priceData.pricesSum() + matchedPrice);
                    priceData.pricesSumSquared(priceData.pricesSumSquared() +
                                               (matchedPrice * matchedPrice));

                    // Calculate new moving average in O(1) time
                    int e = priceData.executions();
                    int n = std::min(e, priceData.maRange());

                    double totalMinusCurrent = priceData.movingAverage() * n;
                    double totalInclCurrent = totalMinusCurrent + matchedPrice;
                    double newMovingAverage = totalInclCurrent / (n + 1);
                    priceData.movingAverage(newMovingAverage);
                }
                else if (priceData.executions() == 0)
                {
                    priceData.movingAverage(matchedPrice);
                }

                priceData.incrementExecutions();
                priceData.demandFactor(updatedDemandFactor(priceData));
            }
            else
            {
                double orderPrice = order->price();

                if (isBid && (!priceData.highestBid() || priceData.highestBid() < orderPrice))
                {
                    priceData.highestBid(orderPrice);
                }

                if (isAsk && (!priceData.lowestAsk() || priceData.lowestAsk() > orderPrice))
                {
                    priceData.lowestAsk(orderPrice);
                }
            }
        });
}

}  // namespace solstice::pricing
