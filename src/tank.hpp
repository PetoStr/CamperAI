#ifndef __TANK_HPP__
#define __TANK_HPP__

#include <BWAPI.h>

#include "actor.hpp"

class Tank: public Actor {
	BWAPI::Unit unit;
public:
	Tank(BWAPI::Unit);
	void act(Context &ctx);
};

#endif
