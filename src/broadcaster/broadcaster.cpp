#include <asset_class.h>
#include <broadcaster.h>
#include <config.h>
#include <order.h>
#include <transaction.h>
#include <types.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>

namespace solstice::broadcaster
{

namespace
{

int64_t timePointToNanos(const TimePoint& tp)
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
}

}  // namespace

// ===================================================================
// WebSocketSession Implementation
// ===================================================================

WebSocketSession::WebSocketSession(tcp::socket&& socket, Broadcaster& broadcaster)
    : d_ws(std::move(socket)), d_broadcaster(broadcaster)
{
}

WebSocketSession::~WebSocketSession() {}

void WebSocketSession::run()
{
    d_ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

    d_ws.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res)
        { res.set(beast::http::field::server, "Solstice-LOB-Broadcaster"); }));

    d_ws.async_accept(beast::bind_front_handler(&WebSocketSession::onAccept, shared_from_this()));
}

void WebSocketSession::onAccept(beast::error_code ec)
{
    if (ec)
    {
        std::cerr << "WebSocket accept error: " << ec.message() << std::endl;
        return;
    }

    d_broadcaster.addSession(shared_from_this());

    std::cout << "[Client connected]" << std::endl;

    d_ws.async_read(d_buffer,
                    beast::bind_front_handler(&WebSocketSession::onRead, shared_from_this()));
}

void WebSocketSession::onRead(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec == websocket::error::closed)
    {
        std::cout << "[Client disconnected]" << std::endl;
        return;
    }

    if (ec)
    {
        std::cerr << "WebSocket read error: " << ec.message() << std::endl;
        return;
    }

    // clear buffer and continue
    d_buffer.consume(d_buffer.size());
    d_ws.async_read(d_buffer,
                    beast::bind_front_handler(&WebSocketSession::onRead, shared_from_this()));
}

void WebSocketSession::send(std::shared_ptr<String const> const& message)
{
    net::post(d_ws.get_executor(),
              beast::bind_front_handler(
                  [self = shared_from_this(), message]()
                  {
                      self->d_writeQueue.push_back(message);

                      // return if already writing
                      if (self->d_writeQueue.size() > 1) return;

                      self->d_ws.async_write(
                          net::buffer(*self->d_writeQueue.front()),
                          beast::bind_front_handler(&WebSocketSession::onWrite, self));
                  }));
}

void WebSocketSession::onWrite(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
    {
        std::cerr << "WebSocket write error: " << ec.message() << std::endl;
        return;
    }

    d_writeQueue.erase(d_writeQueue.begin());

    if (!d_writeQueue.empty())
    {
        d_ws.async_write(net::buffer(*d_writeQueue.front()),
                         beast::bind_front_handler(&WebSocketSession::onWrite, shared_from_this()));
    }
}

// ===================================================================
// Listener Implementation
// ===================================================================

Listener::Listener(net::io_context& ioc, tcp::endpoint endpoint, Broadcaster& broadcaster)
    : d_ioc(ioc), d_acceptor(net::make_strand(ioc)), d_broadcaster(broadcaster)
{
    beast::error_code ec;

    // open the acceptor
    ec = d_acceptor.open(endpoint.protocol(), ec);
    if (ec)
    {
        std::cerr << "Listener open error: " << ec.message() << std::endl;
        return;
    }

    // allow address reuse
    ec = d_acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
        std::cerr << "Listener set_option error: " << ec.message() << std::endl;
        return;
    }

    // bind to server address
    ec = d_acceptor.bind(endpoint, ec);
    if (ec)
    {
        std::cerr << "Listener bind error: " << ec.message() << std::endl;
        return;
    }

    // start listening for connections
    ec = d_acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec)
    {
        std::cerr << "Listener listen error: " << ec.message() << std::endl;
        return;
    }
}

void Listener::run() { doAccept(); }

void Listener::doAccept()
{
    d_acceptor.async_accept(net::make_strand(d_ioc),
                            beast::bind_front_handler(&Listener::onAccept, shared_from_this()));
}

void Listener::onAccept(beast::error_code ec, tcp::socket socket)
{
    if (ec)
    {
        std::cerr << "Listener accept error: " << ec.message() << std::endl;
    }
    else
    {
        // create session and run it
        std::make_shared<WebSocketSession>(std::move(socket), d_broadcaster)->run();
    }

    doAccept();
}

// ===================================================================
// Broadcaster Implementation
// ===================================================================

Broadcaster::Broadcaster(unsigned short port) : d_ioc(1)
{
    d_startTime = timeNow();

    d_ioThread = std::thread([this, port]() { this->run(port); });
    d_broadcastThread = std::thread([this]() { this->broadcastWorker(); });
}

Broadcaster::~Broadcaster()
{
    {
        std::lock_guard<std::mutex> lock(d_queueMutex);
        d_stopBroadcasting = true;
    }
    d_queueCV.notify_all();

    if (d_broadcastThread.joinable())
    {
        d_broadcastThread.join();
    }

    d_ioc.stop();

    if (d_ioThread.joinable())
    {
        d_ioThread.join();
    }
}

void Broadcaster::run(unsigned short port)
{
    auto const address = net::ip::make_address("0.0.0.0");
    auto const endpoint = tcp::endpoint{address, port};

    std::make_shared<Listener>(d_ioc, endpoint, *this)->run();

    d_ioc.run();
}

void Broadcaster::addSession(std::shared_ptr<WebSocketSession> session)
{
    std::lock_guard<std::mutex> lock(d_sessionsMutex);
    d_sessions.push_back(session);
}

void Broadcaster::removeSession(std::shared_ptr<WebSocketSession> session)
{
    std::lock_guard<std::mutex> lock(d_sessionsMutex);
    d_sessions.erase(std::remove_if(d_sessions.begin(), d_sessions.end(),
                                    [&](const std::weak_ptr<WebSocketSession>& weak)
                                    { return weak.expired() || weak.lock() == session; }),
                     d_sessions.end());
}

void Broadcaster::broadcast(const String& message)
{
    {
        std::lock_guard<std::mutex> lock(d_queueMutex);
        d_messageQueue.push(message);
    }
    d_queueCV.notify_one();
}

void Broadcaster::broadcastWorker()
{
    while (true)
    {
        String message;
        {
            std::unique_lock<std::mutex> lock(d_queueMutex);
            d_queueCV.wait(lock,
                           [this]() { return !d_messageQueue.empty() || d_stopBroadcasting; });

            if (d_stopBroadcasting && d_messageQueue.empty())
            {
                break;
            }

            if (!d_messageQueue.empty())
            {
                message = std::move(d_messageQueue.front());
                d_messageQueue.pop();
            }
        }

        if (!message.empty())
        {
            auto sharedMessage = std::make_shared<String>(std::move(message));

            std::lock_guard<std::mutex> lock(d_sessionsMutex);

            for (auto it = d_sessions.begin(); it != d_sessions.end();)
            {
                if (auto session = it->lock())
                {
                    session->send(sharedMessage);
                    ++it;
                }
                else
                {
                    it = d_sessions.erase(it);
                }
            }
        }
    }
}

void Broadcaster::broadcastTrade(const matching::OrderPtr& order)
{
    json msg = {{"type", "trade"},
                {"transaction_id", order->uid()},
                {"symbol", to_string(order->underlying())},
                {"price", order->price()},
                {"quantity", order->qnty()},
                {"timestamp", timePointToNanos(*order->timeOrderFulfilled())}};

    broadcast(msg.dump());
}

void Broadcaster::broadcastBook(const Underlying& underlying,
                                const std::shared_ptr<::solstice::matching::OrderBook>& orderBook)
{
    int count = d_orderCounter.fetch_add(1, std::memory_order_relaxed);

    int broadcastInterval = (*Config::instance()).broadcastInterval();
    if (count % broadcastInterval != 0)
    {
        return;
    }

    auto activeOrdersOpt = orderBook->getActiveOrders(underlying);
    if (!activeOrdersOpt)
    {
        return;
    }

    const auto& activeOrders = activeOrdersOpt->get();

    // Get highest bid (last element in map since it's sorted ascending)
    std::optional<double> bestBid;
    for (auto it = activeOrders.bids.rbegin(); it != activeOrders.bids.rend(); ++it)
    {
        int totalQnty = 0;
        for (const auto& o : it->second)
        {
            totalQnty += o->outstandingQnty();
        }
        if (totalQnty > 0)
        {
            bestBid = it->first;
            break;
        }
    }

    // Get lowest ask (first element in map since it's sorted ascending)
    std::optional<double> bestAsk;
    for (const auto& [price, orders] : activeOrders.asks)
    {
        int totalQnty = 0;
        for (const auto& o : orders)
        {
            totalQnty += o->outstandingQnty();
        }
        if (totalQnty > 0)
        {
            bestAsk = price;
            break;
        }
    }

    json msg = {{"type", "book"},
                {"symbol", to_string(underlying)},
                {"best_bid", bestBid.has_value() ? json(bestBid.value()) : json(nullptr)},
                {"best_ask", bestAsk.has_value() ? json(bestAsk.value()) : json(nullptr)},
                {"timestamp", timePointToNanos(timeNow())}};

    std::unique_lock<std::mutex> lock(d_queueMutex, std::try_to_lock);
    if (lock.owns_lock())
    {
        d_messageQueue.push(msg.dump());
        lock.unlock();
        d_queueCV.notify_one();
    }
}

}  // namespace solstice::broadcaster
