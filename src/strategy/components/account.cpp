#include <account.h>
#include <config.h>

namespace solstice::strategy
{

Account::Account(Config& cfg) { d_balance = cfg.initialBalance(); }

std::expected<Account, String> Account::create()
{
    auto cfg = Config::instance();
    if (!cfg)
    {
        return std::unexpected(cfg.error());
    }

    return Account(*cfg);
}

// getters

int Account::balance() const { return d_balance; }

const std::vector<Position>& Account::positions() const { return d_positions; }

// setters

void Account::balance(int newBalance) { d_balance = newBalance; }

void Account::addPosition(Position pos) { d_positions.emplace_back(pos); }

}  // namespace solstice::strategy
