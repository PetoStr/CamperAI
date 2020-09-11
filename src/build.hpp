#ifndef __BUILD_HPP__
#define __BUILD_HPP__

#include <BWAPI.h>

#include <map>

#include "actor.hpp"
#include "common.hpp"

class Build {
	std::list<Task> build_order;
	std::map<Task *, Actor *> assigned_build_tasks;
	bool building_depot;

	void notify_actor(Task *task);
	bool assign_task(Task *task, Context &ctx, std::map<BWAPI::Unit, Actor *> &actors);
	void check_supply();
	void remove_build_type(BWAPI::UnitType type);
	bool assign_unit_task(Task *task, Context &ctx, std::map<BWAPI::Unit, Actor *> &actors);
	bool assign_research_task(Task *task, Context &ctx, std::map<BWAPI::Unit, Actor *> &actors);
	bool assign_upgrade_task(Task *task, Context &ctx, std::map<BWAPI::Unit, Actor *> &actors);
public:
	void create_wall();
	void create_build_order(Context &ctx);
	void handle_build_order(Context &ctx, std::map<BWAPI::Unit, Actor *> &actors);
	void on_unit_create(Context &ctx, BWAPI::Unit unit);
	void on_unit_complete(Context &ctx, BWAPI::Unit unit);
	void on_unit_morph(Context &ctx, BWAPI::Unit unit);
	void on_unit_destroy(Context &ctx, BWAPI::Unit unit);
	std::list<Task> &get_build_order();
};

#endif
