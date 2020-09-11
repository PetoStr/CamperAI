#ifndef __ARMY_HPP__
#define __ARMY_HPP__

#include <map>
#include <set>

#include "army_unit.hpp"

class Army {
	std::map<BWAPI::Unit, ArmyUnit *> units;
	std::set<BWAPI::Position> enemy_buildings;
	ArmyOrder last_order;
	void remember_enemy();
	void order_all();
	void handle_attack();
	void handle_guard();
	BWAPI::Position get_guard_pos();
	BWAPI::Position get_attack_pos();
	bool some_units_in_pos();
public:
	void init();
	void update();
	void on_unit_complete(BWAPI::Unit unit, ArmyUnit *army_unit);
	void on_unit_destroy(BWAPI::Unit unit);
};

#endif
