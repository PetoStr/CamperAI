#include "science_facility.hpp"

using namespace BWAPI;

ScienceFacility::ScienceFacility(Unit _unit) : unit(_unit)
{
}

void ScienceFacility::act(Context &ctx)
{
}

bool ScienceFacility::assign_task(Context &ctx, Task *task)
{
	Unit sf = this->unit;

	switch (task->type) {
	case TaskType::UNIT:
		if (!sf->getAddon()) {
			if (sf->buildAddon(task->what.unit)) {
				task->state = TaskState::COMPLETE;
				return true;
			} else {
				task->state = TaskState::ADDON_BLOCKED;
				return false;
			}
		}

		return false;
	case TaskType::RESEARCH:
		sf->research(task->what.research);
		task->state = TaskState::COMPLETE;
		return true;
	default:
		break;
	}

	return false;
}
