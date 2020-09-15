#include "scv.hpp"

#include "build.hpp"

using namespace std;
using namespace BWAPI;

const int CONSTRUCT_MAX_DIST = 160;

SCV::SCV(BWAPI::Unit _unit) : unit(_unit)
{
	this->task = nullptr;
	this->state = SCVState::NO_TASK;
}

void SCV::act(Context &ctx)
{
	Unit scv = this->unit;

	TilePosition &where = this->task->where;

	switch (this->state) {
	case SCVState::NO_TASK:
		if (scv->isIdle()) {
			scv->gather(ctx.pick_random_mineral());
		}
		break;
	case SCVState::MOVING:
		if (scv->isIdle()) {
			Position where_pos = Position(where);
			int dist = scv->getDistance(where_pos);
			if (dist <= CONSTRUCT_MAX_DIST) {
				state = SCVState::BUILDING;
			} else {
				scv->move(where_pos);
			}
		}
		break;
	case SCVState::BUILDING:
		if (scv->isConstructing()) {
			this->task->state = TaskState::COMPLETE;
		} else {
			UnitType what = this->task->what.unit;

			bool okhere =
				Broodwar->canBuildHere(where, what, scv, true);

			if (!okhere || !scv->build(what, this->task->where)) {
				this->task->state = TaskState::CANT_BUILD_HERE;
			}
		}
		break;
	}
}

bool SCV::assign_task(Context &ctx, Task *task)
{
	if (task->type != TaskType::UNIT) {
		return false;
	}

	Unit scv = this->unit;

	if ((!scv->isGatheringMinerals() && !scv->isIdle())
			|| !scv->canBuild(task->what.unit, false, false)
			|| this->state != SCVState::NO_TASK) {
		return false;
	}

	scv->move(Position(task->where));
	this->task = task;
	this->state = SCVState::MOVING;

	task->state = TaskState::PENDING_BUILD;

	return true;
}

void SCV::on_task_completion()
{
	this->task = nullptr;
	this->state = SCVState::NO_TASK;
}

void SCV::on_destroy()
{
	if (this->task) {
		this->task->state = TaskState::UNASSIGNED;
		this->task = nullptr;
	}
	this->state = SCVState::NO_TASK;
}
