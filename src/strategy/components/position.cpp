#include <position.h>
#include <position_type.h>

namespace solstice::strategy
{

Position::Position(PositionType posType, double posSize, double entryPrice)
    : d_posType(posType), d_posSize(posSize), d_entryPrice(entryPrice)
{
    d_active = true;
    d_exitPrice = -1;
}

// getters

PositionType Position::posType() const { return d_posType; }

double Position::posSize() const { return d_posSize; }

double Position::entryPrice() const { return d_entryPrice; }

double Position::exitPrice() const { return d_exitPrice; }

double Position::active() const { return d_active; }

// setters

void Position::posSize(int newPosSize) { d_posSize = newPosSize; }

void Position::entryPrice(double newEntryPrice) { d_entryPrice = newEntryPrice; }

void Position::exitPrice(double newExitPrice) { d_exitPrice = newExitPrice; }

void Position::active(double isActive) { d_active = isActive; }

}  // namespace solstice::strategy
