#ifndef __ARMY_HPP__
#define __ARMY_HPP__

#include <map>
#include <memory>
#include <set>

#include "army_unit.hpp"

class Army {
	ArmyUnitMap units;
	std::set<BWAPI::Position> enemy_buildings;
	ArmyOrder last_order;
	void remember_enemy();
	void order_all();
	void handle_attack();
	void handle_guard();
	BWAPI::Position get_guard_pos();
	BWAPI::Position get_attack_pos();
	BWAPI::Position get_possible_enemy_loc();
	int get_units_in_pos();
public:
	void init();
	void update();
	void on_unit_complete(BWAPI::Unit unit, std::shared_ptr<ArmyUnit> army_unit);
	void on_unit_destroy(BWAPI::Unit unit);
};

#endif
