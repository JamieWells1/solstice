#ifndef GREEKS_H
#define GREEKS_H

namespace solstice::pricing
{

class Greeks
{
   public:
    Greeks(double delta, double gamma, double theta, double vega);

    double delta();
    double gamma();
    double theta();
    double vega();

    void delta(double delta);
    void gamma(double gamma);
    void theta(double theta);
    void vega(double vega);

   private:
    double d_delta;
    double d_gamma;
    double d_theta;
    double d_vega;
};

}  // namespace solstice::pricing

#endif  // GREEKS_H
