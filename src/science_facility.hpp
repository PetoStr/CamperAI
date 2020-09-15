#ifndef __SCIENCE_FACILITY_HPP__
#define __SCIENCE_FACILITY_HPP__

#include <BWAPI.h>

#include "actor.hpp"

class ScienceFacility: public Actor {
	BWAPI::Unit unit;
public:
	ScienceFacility(BWAPI::Unit);
	void act(Context &ctx) override;
	bool assign_task(Context &ctx, Task *task) override;
};

#endif
