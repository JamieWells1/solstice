#include <pricing_utils.h>

#include <cmath>

namespace solstice::pricing
{

double N(double x) { return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0))); }

}  // namespace solstice::pricing
