#include <asset_class.h>
#include <config.h>
#include <log_level.h>
#include <logging.h>
#include <market_side.h>
#include <orchestrator.h>
#include <order.h>
#include <order_book.h>
#include <pricer.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace solstice::matching
{

Orchestrator::Orchestrator(Config config, std::shared_ptr<OrderBook> orderBook,
                           std::shared_ptr<Matcher> matcher,
                           std::shared_ptr<pricing::Pricer> pricer,
                           std::optional<broadcaster::Broadcaster>& broadcaster)
    : d_config(config),
      d_orderBook(orderBook),
      d_matcher(matcher),
      d_pricer(pricer),
      d_broadcaster(broadcaster)
{
}

const Config& Orchestrator::config() const { return d_config; }
const std::shared_ptr<OrderBook>& Orchestrator::orderBook() const { return d_orderBook; }
const std::shared_ptr<Matcher>& Orchestrator::matcher() const { return d_matcher; }
const std::shared_ptr<pricing::Pricer>& Orchestrator::pricer() const { return d_pricer; }

std::map<Underlying, std::mutex>& Orchestrator::underlyingMutexes() { return d_underlyingMutexes; }

std::queue<OrderPtr>& Orchestrator::orderProcessQueue() { return d_orderProcessQueue; }

std::expected<OrderPtr, std::string> Orchestrator::generateOrder(int ordersGenerated)
{
    int uid = ordersGenerated;

    auto underlying = getUnderlying(config().assetClass());
    if (!underlying)
    {
        return std::unexpected(underlying.error());
    }

    std::expected<OrderPtr, std::string> order;
    if (config().usePricer())
    {
        order = Order::createWithPricer(pricer(), uid, *underlying);
    }
    else
    {
        order = Order::createWithRandomValues(config(), uid, *underlying);
    }

    if (!order)
    {
        return std::unexpected(order.error());
    }
    return order;
}

bool Orchestrator::processOrder(OrderPtr order)
{
    auto mutexIt = underlyingMutexes().find(order->underlying());
    if (mutexIt != underlyingMutexes().end())
    {
        std::lock_guard<std::mutex> lock(mutexIt->second);
        d_orderBook->addOrderToBook(order);

        auto orderMatched = matcher()->matchOrder(order);

        // Broadcast book after order is processed
        if (d_broadcaster.get().has_value())
        {
            d_broadcaster.get()->broadcastBook(order->underlying(), d_orderBook);
        }

        d_pricer->update(order);

        if (!orderMatched)
        {
            if (config().logLevel() >= LogLevel::DEBUG)
            {
                std::lock_guard<std::mutex> outputLock(d_outputMutex);
                std::cout << "Order: " << order->uid() << " | Status: Idle"
                          << " | Matched with: N/A"
                          << " | Side: " << order->marketSideString()
                          << " | Ticker: " << to_string(order->underlying()) << " | Price: $"
                          << order->price() << " | Qnty: " << order->qnty()
                          << " | Remaining Qnty: " << order->outstandingQnty()
                          << " | Reason: " << orderMatched.error() << "\n";
            }

            return false;
        }
        else
        {
            if (config().logLevel() >= LogLevel::DEBUG)
            {
                std::lock_guard<std::mutex> outputLock(d_outputMutex);
                std::cout << *orderMatched;
            }

            return true;
        }
    }
    else
    {
        // no mutex for this underlying - proceed without locking
        d_orderBook->addOrderToBook(order);

        auto orderMatched = d_matcher->matchOrder(order);

        // Broadcast book after order is processed
        if (d_broadcaster.get().has_value())
        {
            d_broadcaster.get()->broadcastBook(order->underlying(), d_orderBook);
        }

        pricer()->update(order);

        if (!orderMatched)
        {
            if (d_config.logLevel() >= LogLevel::DEBUG)
            {
                std::lock_guard<std::mutex> outputLock(d_outputMutex);
                std::cout << "Order: " << order->uid() << " | Status: Idle"
                          << " | Matched with: N/A"
                          << " | Side: " << order->marketSideString()
                          << " | Ticker: " << to_string(order->underlying()) << " | Price: $"
                          << order->price() << " | Qnty: " << order->qnty()
                          << " | Remaining Qnty: " << order->outstandingQnty()
                          << " | Reason: " << orderMatched.error() << "\n";
            }

            return false;
        }
        else
        {
            if (d_config.logLevel() >= LogLevel::DEBUG)
            {
                std::lock_guard<std::mutex> outputLock(d_outputMutex);
                std::cout << *orderMatched;
            }

            return true;
        }
    }
}

void Orchestrator::pushToQueue(OrderPtr order)
{
    {
        std::lock_guard<std::mutex> lock(d_queueMutex);
        orderProcessQueue().push(order);
    }
    d_queueConditionVar.notify_one();
}

OrderPtr Orchestrator::popFromQueue()
{
    std::unique_lock<std::mutex> lock(d_queueMutex);

    d_queueConditionVar.wait(lock,
                             [this] { return !orderProcessQueue().empty() || d_done.load(); });

    if (orderProcessQueue().empty())
    {
        return nullptr;
    }

    OrderPtr order = orderProcessQueue().front();
    orderProcessQueue().pop();
    return order;
}

void Orchestrator::workerThread(std::atomic<int>& matched, std::atomic<int>& executed)
{
    while (true)
    {
        OrderPtr order = popFromQueue();

        if (!order)
        {
            break;
        }

        if (processOrder(order))
        {
            matched++;
        }
        executed++;
    }
}

void Orchestrator::initialiseUnderlyings(AssetClass assetClass)
{
    switch (assetClass)
    {
        case AssetClass::Equity:
            setUnderlyingsPool(config().underlyingPoolCount(), ALL_EQUITIES);

            orderBook()->initialiseBookAtUnderlyings<Equity>();
            pricer()->initialisePricerEquities();

            for (Equity underlying : underlyingsPool<Equity>())
            {
                underlyingMutexes()[underlying];
            }

            break;
        case AssetClass::Future:
            setUnderlyingsPool(config().underlyingPoolCount(), ALL_FUTURES);

            orderBook()->initialiseBookAtUnderlyings<Future>();
            pricer()->initialisePricerFutures();

            for (Future underlying : underlyingsPool<Future>())
            {
                underlyingMutexes()[underlying];
            }

            break;
        case AssetClass::Option:
            setUnderlyingsPool(config().underlyingPoolCount(), ALL_OPTIONS);

            orderBook()->initialiseBookAtUnderlyings<Option>();
            pricer()->initialisePricerOptions();

            for (Option underlying : underlyingsPool<Option>())
            {
                underlyingMutexes()[underlying];
            }

            break;
        case AssetClass::COUNT:
            break;
    }
}

std::expected<std::pair<int, int>, std::string> Orchestrator::produceOrders()
{
    d_done.store(false);

    std::atomic<int> ordersMatched{0};
    std::atomic<int> ordersExecuted{0};

    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threadPool;

    for (int i = 0; i < numThreads; i++)
    {
        threadPool.emplace_back(&Orchestrator::workerThread, this, std::ref(ordersMatched),
                                std::ref(ordersExecuted));
    }

    size_t i = 0;
    bool infiniteMode = (config().ordersToGenerate() == -1);

    int ordersGenerated = 0;
    while (infiniteMode || i < static_cast<size_t>(config().ordersToGenerate()))
    {
        auto order = generateOrder(ordersGenerated);
        if (!order)
        {
            d_done.store(true);
            d_queueConditionVar.notify_all();
            for (auto& thread : threadPool) thread.join();
            return std::unexpected(order.error());
        }
        pushToQueue(*order);

        if (!infiniteMode)
        {
            i++;
        }
        ordersGenerated++;
    }

    d_done.store(true);
    d_queueConditionVar.notify_all();

    for (auto& worker : threadPool)
    {
        worker.join();
    }

    return std::pair{ordersExecuted.load(), ordersMatched.load()};
}

std::expected<void, std::string> Orchestrator::start(
    std::optional<broadcaster::Broadcaster>& broadcaster)
{
    auto config = Config::instance();

    if (!config)
    {
        return std::unexpected(config.error());
    }

    auto orderBook = std::make_shared<OrderBook>();
    auto matcher = std::make_shared<Matcher>(orderBook);
    auto pricer = std::make_shared<pricing::Pricer>(orderBook);

    Orchestrator orchestrator{*config, orderBook, matcher, pricer, broadcaster};

    orchestrator.initialiseUnderlyings(config->assetClass());

    auto start = timeNow();
    auto result = orchestrator.produceOrders();
    auto end = timeNow();

    if (!result)
    {
        return std::unexpected("An error occured when trying to create orders: " + result.error());
    }

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (config->logLevel() >= LogLevel::INFO)
    {
        std::cout << "\nSUMMARY:"
                  << "\nOrders executed: " << result->first
                  << "\nOrders matched: " << result->second << "\nTime taken: " << duration;
    }

    return {};
}

}  // namespace solstice::matching
