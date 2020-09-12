#ifndef __COMMAND_CENTER_HPP__
#define __COMMAND_CENTER_HPP__

#include <BWAPI.h>

#include "actor.hpp"

class CommandCenter: public Actor {
	BWAPI::Unit unit;
	int max_workers;
public:
	CommandCenter(BWAPI::Unit);
	void act(Context &ctx);
	bool assign_task(Context &ctx, Task *task);
};

#endif
