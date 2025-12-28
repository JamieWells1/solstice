#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include <config.h>
#include <matcher.h>
#include <order.h>
#include <order_book.h>
#include <pricer.h>
#include <broadcaster.h>

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>

namespace solstice::matching
{

class Orchestrator
{
   public:
    static std::expected<void, std::string> start(std::optional<broadcaster::Broadcaster>& broadcaster);

    Orchestrator(Config config, std::shared_ptr<OrderBook> orderBook,
                 std::shared_ptr<Matcher> matcher, std::shared_ptr<pricing::Pricer> pricer,
                 std::optional<broadcaster::Broadcaster>& broadcaster);

    bool processOrder(OrderPtr order);

    const Config& config() const;

    const std::shared_ptr<OrderBook>& orderBook() const;
    const std::shared_ptr<Matcher>& matcher() const;
    const std::shared_ptr<pricing::Pricer>& pricer() const;

    std::map<Underlying, std::mutex>& underlyingMutexes();
    std::queue<OrderPtr>& orderProcessQueue();

   private:
    void initialiseUnderlyings(AssetClass assetClass);
    void pushToQueue(OrderPtr order);
    void workerThread(std::atomic<int>& matched, std::atomic<int>& executed);

    OrderPtr popFromQueue();

    std::expected<std::vector<OrderPtr>, std::string> generateOrders(int ordersGenerated);
    std::expected<std::pair<int, int>, std::string> produceOrders();

    template <typename T>
    void initialiseMutexes(T underlying);

    Config d_config;
    std::shared_ptr<OrderBook> d_orderBook;
    std::shared_ptr<Matcher> d_matcher;
    std::shared_ptr<pricing::Pricer> d_pricer;
    std::reference_wrapper<std::optional<broadcaster::Broadcaster>> d_broadcaster;

    std::map<Underlying, std::mutex> d_underlyingMutexes;
    std::queue<OrderPtr> d_orderProcessQueue;
    std::mutex d_queueMutex;
    std::mutex d_outputMutex;  // protects std::cout from interleaving
    std::condition_variable d_queueConditionVar;
    std::atomic<bool> d_done{false};
};

std::ostream& operator<<(std::ostream& os, ActiveOrders activeOrders);

}  // namespace solstice::matching

#endif  // ORCHESTRATOR_H
