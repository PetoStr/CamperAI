#ifndef __MARINE_HPP__
#define __MARINE_HPP__

#include <BWAPI.h>

#include "actor.hpp"

class Marine: public Actor {
	BWAPI::Unit unit;
public:
	Marine(BWAPI::Unit);
	void act(Context &ctx) override;
};

#endif
