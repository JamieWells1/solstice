# Solstice Pricing Engine

The Solstice Pricing Engine is a dynamic, demand-driven pricing system for generating realistic order flow. It computes market side, price, and quantity for each order based on market microstructure principles including demand factors, spread dynamics, and volatility-adjusted pricing.

> ## [Experiment]
>
> _Placeholder for experiment results_

## Key Features

- Demand factor-driven market side determination with mean reversion.
- Dynamic spread calculation based on moving averages and volatility.
- Support for both equities and futures (with cost-of-carry adjustments).
- Three order types: `AtSpread` (50%), `CrossSpread` (30%), `InsideSpread` (20%).
- Volatility-adjusted quantity sizing based on target notional value.

---

## Architecture

The pricing module consists of several components working together:

| Component            | Purpose                                                |
| -------------------- | ------------------------------------------------------ |
| `Pricer`             | Core engine: computes market side, price, and quantity |
| `EquityPriceData`    | State container for equity-specific price data         |
| `FuturePriceData`    | State container for futures-specific price data        |
| `PricerDepOrderData` | Output struct packaging computed order parameters      |

---

## Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│ Orchestrator.produceOrders()                                │
└─────────────────────────────────┬───────────────────────────┘
                                  │
                    ┌─────────────▼─────────────┐
                    │ Order::createWithPricer() │
                    └─────────────┬─────────────┘
                                  │
                    ┌─────────────▼─────────────┐
                    │ Pricer::computeOrderData()         │
                    │ 1. calculateMarketSide()  │
                    │ 2. calculateMarketPrice()       │
                    │ 3. calculateQnty()        │
                    └─────────────┬─────────────┘
                                  │
                    ┌─────────────▼─────────────┐
                    │ Order Created & Matched   │
                    └─────────────┬─────────────┘
                                  │
                    ┌─────────────▼─────────────┐
                    │ Pricer::update()          │
                    │ Updates price state with: │
                    │ - Execution price         │
                    │ - Bid/ask spread          │
                    │ - Moving average          │
                    │ - Demand factor           │
                    └─────────────┬─────────────┘
                                  │
                            ┌─────▼──────┐
                            │ Next Order │
                            └────────────┘
```

---

## Core Algorithms

### Demand Factor

The demand factor (DF) is a value between `-1.0` and `1.0` that drives market direction:

- **Positive DF**: Bullish pressure (higher probability of bids)
- **Negative DF**: Bearish pressure (higher probability of asks)

**Mean Reversion Logic:**

```
1. Add noise: random [-0.05, 0.05]
2. Calculate price deviation from moving average
3. If price > MA + 1.5σ: DF -= 0.15 (selling pressure)
   If price < MA - 1.5σ: DF += 0.15 (buying pressure)
4. Mean reversion: DF = DF * 0.95
5. Clamp to [-1.0, 1.0]
```

### Market Side Selection

Market side is determined probabilistically using the demand factor:

```
probability = demandFactor²
- High DF² → higher chance of Bid
- Low DF² → higher chance of Ask
```

### Price Calculation

Price is computed based on order type and current spread:

| Order Type   | Probability | Behavior                           |
| ------------ | ----------- | ---------------------------------- |
| AtSpread     | 50%         | Passive: places at best bid/ask    |
| CrossSpread  | 30%         | Aggressive: crosses the spread     |
| InsideSpread | 20%         | Midpoint: places within the spread |

**Equity Spread Calculation:**

- Initial: `price ± (price × 0.2%)`
- Dynamic (after 10 executions): `MA × (0.2% + σ × 0.15%)`

**Futures Spread Calculation:**

- Initial: `price ± (price × 1%)`
- Dynamic: `MA × (0.5% + σ × 1%)`
- Includes carry adjustment: `spot × exp(r × t) - spot` where `r = 5%`

### Quantity Calculation

Quantity is sized to target a notional value of `$10,000`:

```
demandScale = 0.3 + (0.7 × |demandFactor|)
volAdjustment = min(σ, 0.5)
maxQty = (10000 × demandScale) / (price × (1 + volAdjustment))
quantity = random[1, maxQty]
```

---

## State Management

Each underlying maintains its own `PriceData` instance tracking:

| Field              | Description                               |
| ------------------ | ----------------------------------------- |
| `lastPrice`        | Most recent execution price               |
| `highestBid`       | Current best bid                          |
| `lowestAsk`        | Current best ask                          |
| `demandFactor`     | Market direction indicator [-1, 1]        |
| `movingAverage`    | 10-period moving average                  |
| `executions`       | Total execution count                     |
| `pricesSum`        | Sum of prices (for MA calculation)        |
| `pricesSumSquared` | Sum of squared prices (for σ calculation) |

**State Lifecycle:**

```
Initialization:
├─ Random initial price (10-200)
├─ Random demand factor (-1 to 1)
├─ Moving average = initial price
└─ Zero executions

First Trade:
├─ Spread initialized at price ± 0.2%
├─ MA = execution price
└─ Executions = 1

10+ Trades:
├─ Dynamic spread using volatility
├─ Full mean-reversion logic active
└─ Standard deviation computed from history
```

---

## Configuration Constants

### Equity Pricing

| Constant                   | Value | Description                      |
| -------------------------- | ----- | -------------------------------- |
| `INITIAL_SPREAD_PCT`       | 0.2%  | Initial spread width             |
| `BASE_SPREAD_PCT`          | 0.2%  | Base dynamic spread              |
| `VOLATILITY_MULTIPLIER`    | 0.15% | Volatility impact on spread      |
| `MIN_EXEC_FOR_SPREAD_CALC` | 10    | Executions before dynamic spread |
| `TRANSIENT_DRIFT_PCT`      | 2.5%  | Bid/ask oscillation              |

### Futures Pricing

| Constant                | Value | Description                 |
| ----------------------- | ----- | --------------------------- |
| `INITIAL_SPREAD_PCT`    | 1%    | Initial spread width        |
| `BASE_SPREAD_PCT`       | 0.5%  | Base dynamic spread         |
| `VOLATILITY_MULTIPLIER` | 1%    | Volatility impact on spread |
| `RISK_FREE_RATE`        | 5%    | Annual rate for carry calc  |

### Order Type Distribution

| Type         | Probability |
| ------------ | ----------- |
| AtSpread     | 50%         |
| CrossSpread  | 30%         |
| InsideSpread | 20%         |

---

## Integration

The pricer integrates with the orchestrator and order system:

```cpp
// Initialization
auto pricer = std::make_shared<pricing::Pricer>(orderBook);
pricer->addEquitiesToDataMap();  // or addFuturesToDataMap()

// Order creation
Order::createWithPricer(pricer, uid, underlying);

// Post-match update
pricer->update(matchedOrder);
```

Enable/disable via config:

```cpp
Config::usePricer()  // default: true
```

---

## Performance

- **Time Complexity**: O(1) per order (hashmap lookups)
- **Space Complexity**: O(n) where n = number of underlyings
- **Initialization**: Linear in pool size
- **Per-order**: Constant time, no loops
