#ifndef PY_INTERFACE_H
#define PY_INTERFACE_H

#include <config.h>
#include <dispatcher.h>
#include <market_data.h>
#include <strategy.h>

#include <expected>

class PyInterface
{
   public:
    static std::expected<PyInterface, String> establish();

    solstice::strategy::Strategy strategy() const;

    template <typename T>
    std::expected<solstice::strategy::Report, String> orchestrate(RawMarketData& rawData);
};

#endif  // PY_INTERFACE_H
