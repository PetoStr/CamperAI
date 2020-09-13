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
}

bool Factory::assign_task(Context &ctx, Task *task)
{
	Unit factory = this->unit;

	switch (task->type) {
	case TaskType::UNIT:
		if (task->what.unit.isAddon() && !factory->getAddon()) {
			if (factory->buildAddon(task->what.unit)) {
				task->state = TaskState::COMPLETE;
				return true;
			} else {
				task->state = TaskState::ADDON_BLOCKED;
				return false;
			}
		} else if (factory->canTrain(task->what.unit)
			   && factory->train(task->what.unit)) {
			task->state = TaskState::COMPLETE;
			return true;
		}
		return false;
	case TaskType::RESEARCH:
		factory->research(task->what.research);
		task->state = TaskState::COMPLETE;
		return true;
	default:
		break;
	}

	return false;
}
