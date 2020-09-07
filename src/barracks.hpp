#ifndef __BARRACKS_HPP__
#define __BARRACKS_HPP__

#include <BWAPI.h>

#include "actor.hpp"

class Barracks: public Actor {
	BWAPI::Unit unit;
public:
	Barracks(BWAPI::Unit);
	void act(Context &ctx);
};

#endif
