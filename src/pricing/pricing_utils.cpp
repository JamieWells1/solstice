#include <pricing_utils.h>

#include <cmath>

namespace solstice::pricing
{

constexpr double STRIKE_BAND_PERC_OF_SPOT_PRICE = 0.01;

double N(double x) { return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0))); }

// calculate strike price increments for options
double bandIncrement(double spotPrice)
{
    // get exactly 1% of spot price
    double exactBandIncrement = STRIKE_BAND_PERC_OF_SPOT_PRICE * spotPrice;
    double bandSize;

    if (exactBandIncrement < 0.1)
    {
        // 10¢
        bandSize = 0.1;
    }
    // decide where to round to
    else if (exactBandIncrement > 0.1 && exactBandIncrement <= 0.5)
    {
        // round to nearest 
    }

    return bandSize;
}

}  // namespace solstice::pricing
