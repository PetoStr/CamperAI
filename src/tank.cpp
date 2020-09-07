#include "tank.hpp"

using namespace BWAPI;

Tank::Tank(Unit _unit) : unit(_unit)
{
}

void Tank::act(Context &ctx)
{
	Unit tank = this->unit;
	if (!tank->isIdle()) {
		return;
	}

	if (!tank->isSieged()) {
		tank->siege();
	}
}
