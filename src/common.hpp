#ifndef __COMMON_HPP__
#define __COMMON_HPP__

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
	UNIT,
	RESEARCH,
	UPGRADE,
};

struct Task {
	BWAPI::UnitType who;

	union {
		BWAPI::UnitType unit;
		BWAPI::TechType research;
		BWAPI::UpgradeType upgrade;
	} what;
	TaskType type;

	BWAPI::TilePosition where;
	TaskState state;

	BWAPI::Unit assigned_unit;
};

struct Context {
	BWAPI::Unit base;
	BWAPI::Unitset mineral_fields;
	int reserved_minerals;
	int reserved_gas;
	int geysers;

	int get_minerals() const
	{
		return BWAPI::Broodwar->self()->minerals() - reserved_minerals;
	}

	int get_gas() const
	{
		return BWAPI::Broodwar->self()->gas() - reserved_gas;
	}
};

inline BWAPI::Unit pick_random_unit(BWAPI::Unitset &units)
{
	auto it = units.begin();
	std::advance(it, rand() % units.size());
	return *it;
}

#endif

