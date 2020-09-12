#ifndef __ACTOR_HPP__
#define __ACTOR_HPP__

#include "common.hpp"

class Actor {
public:
	virtual ~Actor() {}
	virtual void act(Context &ctx) = 0;

	/*
	 * returns true if the actor will handle the task, otherwise false
	 */
	virtual bool assign_task(Context &ctx, Task *task)
	{
		return false;
	}

	/*
	 * only for buildings
	 */
	virtual void on_task_completion()
	{
	}

	virtual void on_destroy()
	{
	}
};

#endif
