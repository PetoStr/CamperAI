#ifndef __STARPORT_HPP__
#define __STARPORT_HPP__

#include <BWAPI.h>

#include "actor.hpp"

class Starport: public Actor {
	BWAPI::Unit unit;
public:
	Starport(BWAPI::Unit);
	void act(Context &ctx) override;
	bool assign_task(Context &ctx, Task *task) override;
};

#endif
