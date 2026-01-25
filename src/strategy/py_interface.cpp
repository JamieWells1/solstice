#include <config.h>
#include <dispatcher.h>
#include <py_interface.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <sharp_movements.h>
#include <strategy.h>
#include <types.h>
#include "resolution.hpp"

namespace solstice::strategy
{

namespace py = pybind11;

Resolution<PyInterface> PyInterface::establish()
{
    auto config = solstice::Config::instance();
    if (!config)
    {
        return resolution::err(config.error());
    }

    auto pyInterface = PyInterface();

    return pyInterface;
}

template <typename T>
Resolution<solstice::strategy::Report> PyInterface::orchestrate(RawMarketData& rawData)
{
    using namespace solstice::strategy;

    MarketData marketData = MarketData::mapRawInput(rawData);
    const Strategy strategy = solstice::Config::strategy;

    std::unique_ptr<T> logic = Dispatcher::constructStrategy<T>(strategy, marketData);
    Report result = logic->execute();

    return result;
}

PYBIND11_MODULE(py_interface, m)
{
    m.doc() = "Solstice LOB Python bindings";

    // expose Report class
    py::class_<solstice::strategy::Report>(m, "Report")
        .def(py::init<int, int, int, int, double, int, int>(), py::arg("candlesAnalysed"),
             py::arg("tradesCompleted"), py::arg("longTrades"), py::arg("shortTrades"),
             py::arg("pnl"), py::arg("winningTrades"), py::arg("losingTrades"))
        .def_readonly("candlesAnalysed", &solstice::strategy::Report::d_candlesAnalysed)
        .def_readonly("tradesCompleted", &solstice::strategy::Report::d_tradesCompleted)
        .def_readonly("longTrades", &solstice::strategy::Report::d_longTrades)
        .def_readonly("shortTrades", &solstice::strategy::Report::d_shortTrades)
        .def_readonly("pnl", &solstice::strategy::Report::d_pnl)
        .def_readonly("winningTrades", &solstice::strategy::Report::d_winningTrades)
        .def_readonly("losingTrades", &solstice::strategy::Report::d_losingTrades);

    py::class_<PyInterface>(m, "PyInterface")
        .def(py::init<>())
        .def_static("establish",
                    []()
                    {
                        auto result = PyInterface::establish();
                        if (!result)
                        {
                            throw std::runtime_error(result.error());
                        }

                        return *result;
                    })
        .def("orchestrate",
             [](PyInterface& self, RawMarketData& rawData)
             {
                 // Specify the concrete strategy type here
                 auto result = self.orchestrate<solstice::strategy::SharpMovements>(rawData);
                 if (!result)
                 {
                     throw std::runtime_error(result.error());
                 }

                 return *result;
             });
}

}  // namespace solstice::strategy
