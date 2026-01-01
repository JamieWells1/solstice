#include <time_point.h>
#include <types.h>

#include <unordered_map>

namespace solstice
{

TimePoint timeNow() { return std::chrono::system_clock::now(); }

CurrentDate currentDate()
{
    auto now = std::chrono::system_clock::now();
    auto nowTimeT = std::chrono::system_clock::to_time_t(now);
    std::tm* nowTm = std::localtime(&nowTimeT);

    return CurrentDate{nowTm->tm_year + 1900, nowTm->tm_mon + 1, nowTm->tm_mday};
}

int monthToInt(const String& month)
{
    static const std::unordered_map<String, int> monthMap = {
        {"JAN", 1}, {"FEB", 2}, {"MAR", 3}, {"APR", 4},  {"MAY", 5},  {"JUN", 6},
        {"JUL", 7}, {"AUG", 8}, {"SEP", 9}, {"OCT", 10}, {"NOV", 11}, {"DEC", 12}};

    return monthMap.at(month);
}

}  // namespace solstice
