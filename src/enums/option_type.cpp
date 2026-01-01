#include <option_type.h>

#include <ostream>

namespace solstice
{

std::ostream& operator<<(std::ostream& os, const OptionType& optionType)
{
    if (optionType == OptionType::Call)
        os << "Call";
    else
        os << "Put";

    return os;
}

}  // namespace solstice
