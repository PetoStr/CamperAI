#include "starport.hpp"

using namespace BWAPI;

Starport::Starport(Unit _unit) : unit(_unit)
{
	for (auto &unit : Broodwar->self()->getUnits()) {
		if (unit->getType() == UnitTypes::Terran_Bunker) {
			_unit->setRallyPoint(unit);
			break;
		}
	}
}

void Starport::act(Context &ctx)
{
	UnitType bc = UnitTypes::Terran_Battlecruiser;
	if (!this->unit->isTraining() && ctx.has_enough_resources(bc)) {
		this->unit->train(bc);
	}
}

bool Starport::assign_task(Context &ctx, Task *task)
{
	if (task->type != TaskType::UNIT) {
		return false;
	}

	Unit starport = this->unit;
	UnitType what = task->what.unit;

	if ((starport->canBuildAddon() && starport->buildAddon(what))
	    || (starport->canTrain(what) && starport->train(what))) {
		task->state = TaskState::COMPLETE;
		return true;
	}

	return false;
}
