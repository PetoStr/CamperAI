#ifndef __BATTLECRUISER_HPP__
#define __BATTLECRUISER_HPP__

#include <BWAPI.h>

#include "actor.hpp"

class Battlecruiser: public Actor {
	BWAPI::Unit unit;
public:
	Battlecruiser(BWAPI::Unit);
	void act(Context &ctx);
};

#endif
