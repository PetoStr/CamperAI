#ifndef __FACTORY_HPP__
#define __FACTORY_HPP__

#include <BWAPI.h>

#include "actor.hpp"

class Factory: public Actor {
	BWAPI::Unit unit;
public:
	Factory(BWAPI::Unit);
	void act(Context &ctx) override;
	bool assign_task(Context &ctx, Task *task) override;
};

#endif
