#ifndef __TASK_HPP__
#define __TASK_HPP__

#include <BWAPI.h>

enum class TaskState {
	UNASSIGNED,
	PENDING_BUILD,
	COMPLETE,
	CANT_BUILD_HERE,
	ADDON_BLOCKED,
};

inline const char *task_state_str(const TaskState &state)
{
	switch (state) {
		case TaskState::UNASSIGNED:
			return "UNASSIGNED";
		case TaskState::PENDING_BUILD:
			return "PENDING_BUILD";
		case TaskState::COMPLETE:
			return "COMPLETE";
		case TaskState::CANT_BUILD_HERE:
			return "CANT_BUILD_HERE";
		case TaskState::ADDON_BLOCKED:
			return "ADDON_BLOCKED";
	}
	return "UNKNOWN";
}

enum class TaskType {
	NONE,
	UNIT,
	RESEARCH,
	UPGRADE,
};

struct Task {
	BWAPI::UnitType who;

	// very bad but whatever
	union {
		BWAPI::UnitType unit;
		BWAPI::TechType research;
		BWAPI::UpgradeType upgrade;
	} what;
	TaskType type { TaskType::NONE };

	BWAPI::TilePosition where;
	TaskState state { TaskState::UNASSIGNED };

	BWAPI::Unit assigned_unit { nullptr };
};

#endif
