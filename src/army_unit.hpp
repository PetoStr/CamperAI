#ifndef __ARMYUNIT_HPP__
#define __ARMYUNIT_HPP__

#include "actor.hpp"

struct ArmyOrder {
	BWAPI::Order type;
	BWAPI::Position where;
};

class ArmyUnit : public Actor {
public:
	virtual void receive_order(ArmyOrder &order) = 0;
};

typedef std::map<BWAPI::Unit, std::shared_ptr<ArmyUnit>> ArmyUnitMap;

#endif
