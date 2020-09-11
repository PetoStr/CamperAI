#ifndef __BATTLECRUISER_HPP__
#define __BATTLECRUISER_HPP__

#include <BWAPI.h>

#include "army_unit.hpp"

class Battlecruiser: public ArmyUnit {
	BWAPI::Unit unit;
	ArmyOrder order;
	bool new_order;
public:
	Battlecruiser(BWAPI::Unit);
	void act(Context &ctx);
	void receive_order(ArmyOrder &order);
};

#endif
