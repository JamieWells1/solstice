#ifndef GETRANDOM_H
#define GETRANDOM_H

#include <types.h>

#include <random>

namespace solstice
{

class Random
{
   public:
    static String getRandomUid();

    static int getRandomInt(int min, int max);
    static double getRandomDouble(double min, double max);

    static int getRandomBool();

   private:
    static std::random_device rd;
};

}  // namespace solstice

#endif  // GETRANDOM_H
