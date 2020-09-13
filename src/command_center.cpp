#include "command_center.hpp"

using namespace BWAPI;

CommandCenter::CommandCenter(Unit _unit) : unit(_unit)
{
	size_t m = _unit->getUnitsInRadius(256, Filter::IsMineralField).size();
	this->max_workers = 2 * m + m / 2 + m / 3;
}

void CommandCenter::act(Context &ctx)
{
	Player player = Broodwar->self();
	if (!this->unit->isTraining() && ctx.get_minerals() >= 50
	    && player->allUnitCount(UnitTypes::Terran_SCV) < this->max_workers) {
		this->unit->train(UnitTypes::Terran_SCV);
	}
}

bool CommandCenter::assign_task(Context &ctx, Task *task)
{
	if (task->type != TaskType::UNIT) {
		return false;
	}

	Unit cc = this->unit;
	UnitType what = task->what.unit;

	if (what.isAddon() && !cc->getAddon()) {
		if (cc->buildAddon(what)) {
			task->state = TaskState::COMPLETE;
			return true;
		} else {
			task->state = TaskState::ADDON_BLOCKED;
			return false;
		}
	} else if (cc->canTrain(what) && cc->train(what)) {
		task->state = TaskState::COMPLETE;
		return true;
	}

	return false;
}
