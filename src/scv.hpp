#ifndef __SCV_HPP__
#define __SCV_HPP__

#include <BWAPI.h>

#include "actor.hpp"

enum class SCVState {
	NO_TASK,
	MOVING,
	BUILDING,
};

class SCV: public Actor {
	BWAPI::Unit unit;
	Task *task;
	SCVState state;
public:
	SCV(BWAPI::Unit);
	void act(Context &ctx);
	bool assign_task(Context &ctx, Task *task);
	void on_task_completion();
	void on_destroy();
};

#endif
