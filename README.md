# Solstice Exchange

> **_Matching Engine, Pricing Engine, and Algorithmic Trading Strategy Bundled Into One Shared Library: Solstice_**

Solstice is a modular C++23 library which provides high-performance electronic trading systems. It integrates core exchange infrastructure such as a limit order book, order matcher, pricing simulator, and algorithmic strategy executor, all under a unified parent namespace.

- `solstice::matching` — Order book, matcher, order processor.
- `solstice::pricing` — Market simulation and pricing feed (WIP).
- `solstice::strategy` — Automated strategy execution (WIP).

---

## Performance

### Latest (v0.2.0, Multi-threaded)

> 🚀 Peak Throughput: **523,560 orders/sec** @ v0.2.0 on M4 MacBook Pro (multi-threaded)

| Orders Executed | Execution Time (ms) | Throughput (orders/sec) |
| --------------- | ------------------- | ----------------------- |
| 100,000         | 191                 | 523,560                 |

See [`BENCHMARK_HISTORY.md`](BENCHMARK_HISTORY.md) for all historical runs, and [`src/matching/README.md`](src/matching/README.md) for full implementation details.

---

## Status & Roadmap

| Component | Status      | Next Steps                                                                                         |
| --------- | ----------- | -------------------------------------------------------------------------------------------------- |
| Matching  | Complete    | Minor optimizations, extended test coverage                                                        |
| Pricing   | Complete    | - Futures pricing (cost of carry model)<br>- Options pricing (Black-Scholes, Greeks calculation)   |
| Strategy  | In progress | Algorithmic trading strategy implementation<br>Initial focus: delta-neutral / volatility arbitrage |

### Roadmap Overview

1. **Matching Engine** — Foundation complete, ongoing refinements
2. **Futures Pricing** — Simple cost of carry model, integration with order book
3. **Options Pricing** — Black-Scholes implementation with full Greeks
4. **Trading Strategy** — Automated strategy executor leveraging derivatives pricing
5. **Extended Derivatives** — Additional asset classes and pricing models

---

## Getting Started

### Building the Project

Solstice uses CMake for build configuration. To build:

```bash
mkdir build && cd build && cmake .. && cmake --build .
```

The main executable will be output to `build/bin/solstice`.

### Configuration

All runtime configuration is managed via [src/config/config.h](src/config/config.h). Key parameters:

**Order Generation & Matching:**

- `d_ordersToGenerate` — Number of orders to generate (default: 10000)
- `d_underlyingPoolCount` — Number of distinct tickers/symbols (default: 8)
- `d_minQnty` / `d_maxQnty` — Quantity range for random orders (default: 1-20)
- `d_minPrice` / `d_maxPrice` — Price range for random orders (default: 9.0-10.0)
- `d_usePricer` — Use pricing engine for order generation (default: true)

**Broadcaster:**

- `d_enableBroadcaster` — Enable WebSocket broadcasting (default: false)
- `d_broadcastInterval` — Broadcast 1 in N orders to reduce traffic (default: 10)

**Logging:**

- `d_logLevel` — Set log verbosity: `TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR` (default: INFO)

**Backtesting:**

- `d_initialBalance` — Starting capital for strategy backtests (default: 10000)

### Running the Matching Engine

After building, run the main executable:

```bash
./build/bin/solstice
```

The program will:

1. Initialize the order book and (optionally) start the WebSocket broadcaster on port 8080
2. Wait for user input to begin order flow
3. Process orders according to config parameters
4. Display execution statistics

If the broadcaster is enabled, it streams real-time market data (trades, orders, book updates) via WebSocket in JSON format.

### Running the Backtesting Engine

The backtesting engine executes trading strategies against historical market data fetched via yfinance. To run:

```bash
./backtest.sh
```

Optional rebuild flag if C++ bindings have changed:

```bash
./backtest.sh --rebuild
```

The script will:

1. Activate the Python virtual environment
2. Fetch historical price data for the configured ticker
3. Execute the configured strategy (see [src/config/config.h](src/config/config.h))
4. Output trade statistics and performance metrics

Strategy configuration is set via `Config::strategy` in [src/config/config.h](src/config/config.h). Current strategies include `SharpMovements` and others defined in the strategy namespace.

---

## Modules

- [`src/matching/README.md`](src/matching/README.md) — Matching engine design and benchmarks.
- [`src/broadcaster/README.md`](src/broadcaster/README.md) — WebSocket broadcaster implementation and performance.
- [`src/pricing/README.md`](src/pricing/README.md) — Pricing engine spec (placeholder).
- [`src/strategy/README.md`](src/strategy/README.md) — Strategy engine spec (placeholder).
- [`BENCHMARK_HISTORY.md`](BENCHMARK_HISTORY.md) — Historical performance data and trends.

---

This repo serves as a single, extensible foundation for simulating and testing real-world trading systems in a controlled, performant environment.

MIT License
