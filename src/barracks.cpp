#include "barracks.hpp"

using namespace BWAPI;

const int MARINES_PER_BUNKER = 4;

Barracks::Barracks(Unit _unit) : unit(_unit)
{
}

void Barracks::act(Context &ctx)
{
	Player player = Broodwar->self();
	Unit barracks = this->unit;
	UnitType marine = UnitTypes::Terran_Marine;
	int bunkers = player->allUnitCount(UnitTypes::Terran_Bunker);
	int marines = player->allUnitCount(marine);

	if (marines < bunkers * MARINES_PER_BUNKER) {
		bool training = barracks->isTraining();
		if (!training && ctx.has_enough_resources(marine)) {
			this->unit->train(marine);
		}
	}
}
