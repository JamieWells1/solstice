#ifndef OPTION_TYPE_H
#define OPTION_TYPE_H

#include <cstdint>
#include <ostream>

namespace solstice
{

enum class OptionType : uint8_t
{
    Call = 1,
    Put = 0
};

std::ostream& operator<<(std::ostream& os, const OptionType& optionType);

}  // namespace solstice

#endif // OPTION_TYPE_H
