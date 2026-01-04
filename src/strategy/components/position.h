#ifndef POSITION_H
#define POSITION_H

#include <position_type.h>
#include <time_point.h>

namespace solstice::strategy
{

struct Position
{
   public:
    Position(PositionType posType, double posSize, double entryPrice);

    PositionType posType() const;
    double posSize() const;
    double entryPrice() const;
    double exitPrice() const;
    double active() const;

    void posSize(int newPosSize);
    void entryPrice(double newEntryPrice);
    void exitPrice(double newExitPrice);
    void active(double isActive);

   private:
    PositionType d_posType;
    double d_posSize;
    double d_entryPrice;
    double d_exitPrice;
    bool d_active;
};

}  // namespace solstice::strategy

#endif  // POSITION_H
