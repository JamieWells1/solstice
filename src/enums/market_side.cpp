#include <market_side.h>

#include <ostream>

namespace solstice
{

std::ostream& operator<<(std::ostream& os, const MarketSide& marketSide)
{
    if (marketSide == MarketSide::Bid)
        os << "Bid";
    else
        os << "Ask";

    return os;
}
}  // namespace solstice
