#include <broadcaster.h>
#include <config.h>
#include <orchestrator.h>
#include <order_book.h>

#include <iostream>
#include <optional>

#include <log_level.h>

using namespace solstice;

int main()
{
    auto config = Config::instance();

    if (!config)
    {
        std::cout << "\n[FATAL]: " << config.error() << std::endl;
        return -1;
    }

    std::optional<broadcaster::Broadcaster> broadcaster;
    if ((*config).enableBroadcaster())
    {
        broadcaster.emplace(8080);
        std::cout << "Broadcaster started on port 8080.\n" << std::endl;
    }

    std::string choice;
    std::cout << "Enter any key to start order flow.\n";
    std::cin >> choice;

    if (!choice.empty())
    {
        auto response = matching::Orchestrator::start(broadcaster);

        if (!response && (*config).logLevel() <= LogLevel::ERROR)
        {
            std::cout << "\n[FATAL]: " << response.error() << std::endl;
            return -1;
        }

        std::cout << std::endl;
        return 0;
    }

    return -1;
}
