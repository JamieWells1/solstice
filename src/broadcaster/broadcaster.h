#ifndef BROADCASTER_H
#define BROADCASTER_H

#include <asset_class.h>
#include <order_book.h>
#include <time_point.h>
#include <transaction.h>
#include <types.h>

#include <atomic>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <condition_variable>
#include <json.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace solstice::broadcaster
{

using json = nlohmann::json;
using solstice::Order;
using solstice::Underlying;

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class WebSocketSession;

class Broadcaster
{
   public:
    Broadcaster(unsigned short port = 8080);
    ~Broadcaster();

    // Disable copy/move
    Broadcaster(const Broadcaster&) = delete;
    Broadcaster& operator=(const Broadcaster&) = delete;

    void broadcastTrade(const ::solstice::matching::OrderPtr& order);
    void broadcastBook(const Underlying& underlying,
                       const std::shared_ptr<::solstice::matching::OrderBook>& orderBook);

    // Session management (called by sessions)
    void addSession(std::shared_ptr<WebSocketSession> session);
    void removeSession(std::shared_ptr<WebSocketSession> session);

   private:
    void broadcast(const String& message);
    void run(unsigned short port);
    void broadcastWorker();  // Background thread for async broadcasting

    net::io_context d_ioc;
    std::thread d_ioThread;
    std::thread d_broadcastThread;

    std::mutex d_sessionsMutex;
    std::vector<std::weak_ptr<WebSocketSession>> d_sessions;

    // Message queue for async broadcasting
    std::queue<String> d_messageQueue;
    std::mutex d_queueMutex;
    std::condition_variable d_queueCV;
    std::atomic<bool> d_stopBroadcasting{false};

    // Counter for sampling broadcasts
    std::atomic<int> d_orderCounter{0};

    // Timer for measuring broadcast duration (DEBUG only)
    TimePoint d_startTime;
};

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
   public:
    explicit WebSocketSession(tcp::socket&& socket, Broadcaster& broadcaster);
    ~WebSocketSession();

    void run();
    void send(std::shared_ptr<String const> const& message);

   private:
    void onAccept(beast::error_code ec);
    void onRead(beast::error_code ec, std::size_t bytes_transferred);
    void onWrite(beast::error_code ec, std::size_t bytes_transferred);

    websocket::stream<beast::tcp_stream> d_ws;
    Broadcaster& d_broadcaster;
    beast::flat_buffer d_buffer;
    std::vector<std::shared_ptr<String const>> d_writeQueue;
};

class Listener : public std::enable_shared_from_this<Listener>
{
   public:
    Listener(net::io_context& ioc, tcp::endpoint endpoint, Broadcaster& broadcaster);
    void run();

   private:
    void doAccept();
    void onAccept(beast::error_code ec, tcp::socket socket);

    net::io_context& d_ioc;
    tcp::acceptor d_acceptor;
    Broadcaster& d_broadcaster;
};

}  // namespace solstice::broadcaster

#endif  // BROADCASTER_H
