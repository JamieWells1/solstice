#include <greeks.h>

namespace solstice::pricing
{

Greeks::Greeks(double delta, double gamma, double theta, double vega)
    : d_delta(delta), d_gamma(gamma), d_theta(theta), d_vega(vega)
{
}

double Greeks::delta() { return d_delta; }
double Greeks::gamma() { return d_gamma; }
double Greeks::theta() { return d_theta; }
double Greeks::vega() { return d_vega; }

void Greeks::delta(double delta) { d_delta = delta; }
void Greeks::gamma(double gamma) { d_gamma = gamma; }
void Greeks::theta(double theta) { d_theta = theta; }
void Greeks::vega(double vega) { d_vega = vega; }

}  // namespace solstice::pricing
