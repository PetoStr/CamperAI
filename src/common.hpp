#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <BWAPI.h>

const int MAX_SUPPLY = 400;

struct Context {
	BWAPI::Unit base;
	BWAPI::Unitset mineral_fields;
	int reserved_minerals;
	int reserved_gas;
	int geysers;

	static BWAPI::Unit pick_random_unit(BWAPI::Unitset &units)
	{
		auto it = units.begin();
		std::advance(it, rand() % units.size());
		return *it;
	}

	int get_minerals() const
	{
		return BWAPI::Broodwar->self()->minerals() - reserved_minerals;
	}

	int get_gas() const
	{
		return BWAPI::Broodwar->self()->gas() - reserved_gas;
	}

	bool has_enough_resources(int req_minerals, int req_gas) const
	{
		return get_minerals() >= req_minerals && get_gas() >= req_gas;
	}

	bool has_enough_resources(BWAPI::UnitType type) const
	{
		return has_enough_resources(type.mineralPrice(), type.gasPrice());
	}

	BWAPI::Unit pick_random_mineral()
	{
		return pick_random_unit(mineral_fields);
	}
};

#endif
