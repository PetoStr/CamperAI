#include "scv.hpp"

#include "build.hpp"

using namespace std;
using namespace BWAPI;

const int BUILD_DELAY_FRAMES = 5;

SCV::SCV(BWAPI::Unit _unit) : unit(_unit)
{
	this->task = nullptr;
	this->state = SCVState::NO_TASK;
}

void SCV::act(Context &ctx)
{
	Unit scv = this->unit;

	switch (this->state) {
		case SCVState::NO_TASK:
			if (scv->isIdle()) {
				scv->gather(pick_random_unit(ctx.mineral_fields));
			}
			break;
		case SCVState::MOVING:
			if (scv->isIdle()) {
				state = SCVState::BUILDING;
			}
			break;
		case SCVState::BUILDING:
			if (scv->isConstructing()) {
				this->task->state = TaskState::COMPLETE;
			} else {
				UnitType what = this->task->what.unit;
				Broodwar << "Building " << what << endl;

				bool okhere = Broodwar->canBuildHere(this->task->where,
								     what,
								     scv,
								     true);


				// TODO handle all build failures
				if (!okhere || !scv->build(what, this->task->where)) {
					Broodwar << "build failed" << endl;
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

	if (!scv->isGatheringMinerals()
	    || this->state != SCVState::NO_TASK) {
		return false;
	}

	Broodwar << "Moving" << endl;
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
