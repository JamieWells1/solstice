#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <config.h>
#include <position.h>

#include <expected>

namespace solstice::strategy
{

class Account
{
   public:
    static std::expected<Account, String> create();

    int balance() const;
    const std::vector<Position>& positions() const;

    void balance(int newBalance);
    void addPosition(Position pos);

   private:
    Account(Config& cfg);

    int d_balance;
    std::vector<Position> d_positions;
};

}  // namespace solstice::strategy

#endif  // ACCOUNT_H
