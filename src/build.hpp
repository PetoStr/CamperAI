#ifndef __BUILD_HPP__
#define __BUILD_HPP__

#include <BWAPI.h>

#include <map>
#include <memory>

#include "actor.hpp"
#include "common.hpp"

class Build {
	std::list<Task> build_order;
	std::map<Task *, std::shared_ptr<Actor>> assigned_build_tasks;
	bool building_depot;

	void notify_actor(Task *task);
	bool assign_task(Task *task, Context &ctx, ActorMap &actors);
	void check_supply();
	void remove_build_type(BWAPI::UnitType type);
	bool assign_unit_task(Task *task, Context &ctx, ActorMap &actors);
	bool assign_research_task(Task *task, Context &ctx, ActorMap &actors);
	bool assign_upgrade_task(Task *task, Context &ctx, ActorMap &actors);
	BWAPI::TilePosition find_build_tile(BWAPI::UnitType type);
	BWAPI::TilePosition fix_build_tile(BWAPI::Unit builder,
			BWAPI::UnitType type, BWAPI::TilePosition origin);
public:
	void create_wall();
	void create_build_order(Context &ctx);
	void handle_build_order(Context &ctx, ActorMap &actors);
	void on_unit_create(Context &ctx, BWAPI::Unit unit);
	void on_unit_complete(Context &ctx, BWAPI::Unit unit);
	void on_unit_morph(Context &ctx, BWAPI::Unit unit);
	void on_unit_destroy(Context &ctx, BWAPI::Unit unit);
	const std::list<Task> &get_build_order();
};

#endif
