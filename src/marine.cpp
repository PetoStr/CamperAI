#include "marine.hpp"

using namespace BWAPI;

Marine::Marine(Unit _unit) : unit(_unit)
{
}

void Marine::act(Context &ctx)
{
	if (!this->unit->isIdle()) return;

	Player player = Broodwar->self();

	// TODO: find a better alternative
	for (auto &unit : player->getUnits()) {
		if (unit->getType() != UnitTypes::Terran_Bunker
			|| !unit->isCompleted()) {
			continue;
		}

		if (unit->getLoadedUnits().size() < 4) {
			unit->load(this->unit);
			break;
		}
	}
}
