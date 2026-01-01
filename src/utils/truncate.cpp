#include <truncate.h>
#include <types.h>

namespace solstice::matching
{

String truncate(String str, size_t length)
{
    if (str.length() > length)
    {
        return str.substr(0, length) + "...";
    }

    return str;
}

}  // namespace solstice::matching
