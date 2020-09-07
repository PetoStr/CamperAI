#include "barracks.hpp"

using namespace BWAPI;

Barracks::Barracks(Unit _unit) : unit(_unit)
{
}

void Barracks::act(Context &ctx)
{
	Player player = Broodwar->self();
	int bunkers = player->allUnitCount(UnitTypes::Terran_Bunker);
	if (player->allUnitCount(UnitTypes::Terran_Marine) >= bunkers * 4) {
		return;
	}

	if (!this->unit->isTraining() && ctx.get_minerals() >= 50) {
		this->unit->train(UnitTypes::Terran_Marine);
	}
}
