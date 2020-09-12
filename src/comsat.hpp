#ifndef __COMSAT_HPP__
#define __COMSAT_HPP__

#include <BWAPI.h>

#include "actor.hpp"

class Comsat: public Actor {
	BWAPI::Unit unit;
	bool is_ally_close(BWAPI::Unit enemy);
	int wait_frames;
public:
	Comsat(BWAPI::Unit);
	void act(Context &ctx);
};

#endif
