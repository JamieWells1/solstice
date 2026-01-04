# Solstice WebSocket Broadcaster

Real-time market data broadcasting component built on Boost.Beast and Boost.Asio. Streams trade executions, order updates, and book changes to connected WebSocket clients with async message queuing and multi-threaded session management.

> ## [See Benchmark History]([link to benchmark history])

## Key Features

- WebSocket server broadcasting trades, orders, and book updates in JSON format
- Async message queuing with dedicated broadcast worker thread to prevent blocking
- Thread-safe session management with weak pointer cleanup
- Configurable order broadcast sampling via `broadcastInterval` to reduce traffic
- Non-blocking broadcast with `try_to_lock` mechanism for high-frequency order updates

---

## Local Execution

1. Set `d_enableBroadcaster` to `true` in `config.h`

2. Run the executable:

```bash
./build/bin/solstice
```

3. In a separate terminal, connect with the WebSocket client:

```bash
python3 websocket_client.py [TICKER]
```

Examples:

```bash
python3 websocket_client.py        # Show all tickers
python3 websocket_client.py AAPL   # Filter to AAPL only
python3 websocket_client.py MSFT   # Filter to MSFT only
```

The client receives book updates in JSON format:

```json
{
  "type": "book",
  "symbol": "AAPL",
  "best_bid": 149.5,
  "best_ask": 150.25,
  "timestamp": 1234567890
}
```

Available tickers are defined in `equity.h` and `future.h`.

---

## Benchmarks

### How orders broadcasted per connection affects throughput

This benchmark will test how orders broadcasted per connection affects throughput, starting with the broadcaster enabled, moving up to 5 concurrent connections.

My hypothesis is that there will be a negligible fixed latency when the broadcaster is on compared to when it is turned off, and then a larger, steady increase in latency as more connections are added. This is because the heavy lifting resides in the multi-threaded session management and message distribution, which I suspect increases linearly with number of connections.

**Constants:**

- Tickers: 10
- Orders: 50,000
- Broadcast Interval: 1
- Tests per entry: 3

---

### Plotting Connections Against Throughput

| Broadcaster | Connections | Throughput (ms) |
| ----------- | ----------- | --------------- |
| False       | 0           | 184             |
| True        | 0           | 434             |
| True        | 1           | 503             |
| True        | 2           | 528             |
| True        | 3           | 543             |
| True        | 4           | 556             |
| True        | 5           | 564             |

Surprisingly, the 'throughput tax' was on the broadcaster being enabled rather than the number of connections. I believe this is because although no connections were established in the second row, the multi-threaded logic to send to connections was still executed. After that, there is a taxing jump from 0 to 1 connections, indicating that some broadcasting logic is specific to there being at least one connection. A logarithmic relationship with throughput is then observered as the number of connections increases.

Plotting latency in deciseconds against number of connections on a graph and performing a `log(x+1)` shift on the data points, we can see a clear logarithmic relationship, with the equation being `y = 4.42972 + 0.714627(ln(x))`, and an `R²` value of `0.975`.

![Broadcaster Latency Line Graph](/assets/v2.2.0_broadcaster_latency.png)
