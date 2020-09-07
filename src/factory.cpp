#include "factory.hpp"

using namespace BWAPI;

Factory::Factory(Unit _unit) : unit(_unit)
{
	for (auto &unit : Broodwar->self()->getUnits()) {
		if (unit->getType() == UnitTypes::Terran_Bunker) {
			_unit->setRallyPoint(unit);
			break;
		}
	}
}

void Factory::act(Context &ctx)
{
	/*if (!this->unit->isTraining() && ctx.get_minerals() >= 150
	    && ctx.get_gas() >= 50) {
		this->unit->train(UnitTypes::Terran_Siege_Tank_Tank_Mode);
	}*/
}

bool Factory::assign_task(Context &ctx, Task *task)
{
	Unit factory = this->unit;

	switch (task->type) {
	case TaskType::UNIT:
		if ((factory->canBuildAddon() && factory->buildAddon(task->what.unit))
		    || (factory->canTrain(task->what.unit) && factory->train(task->what.unit))) {
			task->state = TaskState::COMPLETE;
			return true;
		}
		return false;
	case TaskType::RESEARCH:
		factory->research(task->what.research);
		task->state = TaskState::COMPLETE;
		return true;
	}

	return false;
}
