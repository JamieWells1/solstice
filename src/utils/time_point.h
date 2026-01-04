#ifndef TIMEPOINT_H
#define TIMEPOINT_H

#include <types.h>

#include <chrono>

namespace solstice
{

using TimePoint = std::chrono::system_clock::time_point;

struct CurrentDate
{
    int year;
    int month;
    int day;
};

TimePoint timeNow();

CurrentDate currentDate();

int monthToInt(const String& month);

}  // namespace solstice

#endif  // TIMEPOINT_H
