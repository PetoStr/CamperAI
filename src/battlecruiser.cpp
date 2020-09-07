#include "battlecruiser.hpp"

using namespace BWAPI;

Battlecruiser::Battlecruiser(Unit _unit) : unit(_unit)
{
}

void Battlecruiser::act(Context &ctx)
{
	Unit bc = this->unit;
	if (!bc->isIdle()) return;

	bc->attack(Position(rand() % 3200, rand() % 3200));
}
