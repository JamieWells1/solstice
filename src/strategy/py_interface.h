#ifndef PY_INTERFACE_H
#define PY_INTERFACE_H

#include <config.h>
#include <dispatcher.h>
#include <market_data.h>
#include <strategy.h>
#include <types.h>

#include <expected>

namespace solstice::strategy
{

class PyInterface
{
   public:
    static std::expected<PyInterface, String> establish();

    solstice::strategy::Strategy strategy() const;

    template <typename T>
    std::expected<solstice::strategy::Report, String> orchestrate(RawMarketData& rawData);
};

}  // namespace solstice::strategy

#endif  // PY_INTERFACE_H
