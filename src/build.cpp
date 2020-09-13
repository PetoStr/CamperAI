#include "build.hpp"

#include <BWEB.h>

using namespace std;
using namespace BWAPI;

static TilePosition find_closest_geyser()
{
	TilePosition center = BWEB::Map::getMainTile();
	Unitset geysers = Broodwar->getGeysers();
	double best_dist = numeric_limits<double>::infinity();
	TilePosition best_pos;
	for (auto &geyser : geysers) {
		double dist = center.getDistance(geyser->getTilePosition());
		if (dist < best_dist) {
			best_pos = geyser->getTilePosition();
			best_dist = dist;
		}
	}

	return best_pos;
}

// try to find suitable position by testing aroud quad of size `dist`
static TilePosition try_expand(Unit builder, TilePosition origin, UnitType type, int dist)
{
	int width = type.tileWidth();
	int height = type.tileHeight();
	for (int y = -dist; y <= dist; y++) {
		for (int m = -1; m <= 1; m += 2) {
			TilePosition pos = origin + TilePosition(m * dist, y);
			if (Broodwar->canBuildHere(pos, type, builder)
			    && BWEB::Map::isUsed(pos, width, height) == UnitTypes::None) {
				return pos;
			}

			pos = origin + TilePosition(y, m * dist);
			if (Broodwar->canBuildHere(pos, type, builder)
			    && BWEB::Map::isUsed(pos, width, height) == UnitTypes::None) {
				return pos;
			}
		}
	}

	return TilePositions::Invalid;
}

TilePosition Build::fix_build_tile(Unit builder, UnitType type, TilePosition origin)
{
	if (type == UnitTypes::Terran_Refinery) {
		return origin;
	}

	TilePosition pos = TilePositions::Invalid;

	int dist = 0;
	while (!pos && dist < 50) {
		dist++;
		pos = try_expand(builder, origin, type, dist);
	}

	if (pos != TilePositions::Invalid) {
		return pos;
	} else {
		return this->find_build_tile(type);
	}
}

TilePosition Build::find_build_tile(UnitType type)
{
	TilePosition pos;
	if (type == UnitTypes::Terran_Missile_Turret
	    || type == UnitTypes::Terran_Bunker) {
		pos = BWEB::Map::getDefBuildPosition(type);
	} else if (type.isRefinery()) {
		pos = find_closest_geyser();
	} else {
		pos = BWEB::Map::getBuildPosition(type);
	}

	if (!Broodwar->canBuildHere(pos, type)) {
		pos = this->fix_build_tile(nullptr, type, pos);
	}

	return pos;
}

void Build::create_wall()
{
	vector<UnitType> buildings {
		UnitTypes::Terran_Supply_Depot,
		UnitTypes::Terran_Barracks,
	};
	vector<UnitType> defenses {
		UnitTypes::Terran_Bunker,
		UnitTypes::Terran_Bunker,
		UnitTypes::Terran_Bunker,
		UnitTypes::Terran_Missile_Turret,
	};

	BWEB::Walls::createWall(buildings, BWEB::Map::getMainArea(),
			BWEB::Map::getMainChoke(), UnitTypes::None,
			defenses, true, false);
}

void Build::create_build_order(Context &ctx)
{

	BWEB::Wall *wall = BWEB::Walls::getClosestWall(BWEB::Map::getMainTile());

	if (wall == nullptr) {
		Broodwar << "No wall has been found" << endl;
		return;
	}

	auto medium_tiles = wall->getMediumTiles();
	auto large_tiles = wall->getLargeTiles();
	auto defense_tiles = wall->getDefenses();
	auto medium_it = medium_tiles.begin();
	auto large_it = large_tiles.begin();
	auto defense_it = defense_tiles.begin();

	if (medium_it != medium_tiles.end()) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Supply_Depot },
			.type = TaskType::UNIT,
			.where = *medium_it,
			.state = TaskState::UNASSIGNED,
		});
		++medium_it;
	}
	this->building_depot = true;
	if (large_it != large_tiles.end()) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Barracks },
			.type = TaskType::UNIT,
			.where = *large_it,
			.state = TaskState::UNASSIGNED,
		});
		++large_it;
	}
	if (ctx.geysers > 0) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Refinery },
			.type = TaskType::UNIT,
			.where = TilePositions::None,
			.state = TaskState::UNASSIGNED,
		});
	}
	if (defense_it != defense_tiles.end()) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Bunker },
			.type = TaskType::UNIT,
			.where = *defense_it,
			.state = TaskState::UNASSIGNED,
		});
		++defense_it;
	}
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_SCV,
		.what = { .unit = UnitTypes::Terran_Factory },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	if (defense_it != defense_tiles.end()) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Bunker },
			.type = TaskType::UNIT,
			.where = *defense_it,
			.state = TaskState::UNASSIGNED,
		});
		++defense_it;
	}
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Factory,
		.what = { .unit = UnitTypes::Terran_Machine_Shop },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Factory,
		.what = { .unit = UnitTypes::Terran_Siege_Tank_Tank_Mode },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Machine_Shop,
		.what = { .research = TechTypes::Tank_Siege_Mode },
		.type = TaskType::RESEARCH,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_SCV,
		.what = { .unit = UnitTypes::Terran_Engineering_Bay },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	if (defense_it != defense_tiles.end()) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Bunker },
			.type = TaskType::UNIT,
			.where = *defense_it,
			.state = TaskState::UNASSIGNED,
		});
		++defense_it;
	}
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Factory,
		.what = { .unit = UnitTypes::Terran_Siege_Tank_Tank_Mode },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	if (defense_it != defense_tiles.end()) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Missile_Turret },
			.type = TaskType::UNIT,
			.where = *defense_it,
			.state = TaskState::UNASSIGNED,
		});
		++defense_it;
	}
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Factory,
		.what = { .unit = UnitTypes::Terran_Siege_Tank_Tank_Mode },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	if (ctx.geysers > 1) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Refinery },
			.type = TaskType::UNIT,
			.where = TilePositions::None,
			.state = TaskState::UNASSIGNED,
		});
	}
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_SCV,
		.what = { .unit = UnitTypes::Terran_Academy },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_SCV,
		.what = { .unit = UnitTypes::Terran_Starport },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_SCV,
		.what = { .unit = UnitTypes::Terran_Starport },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Command_Center,
		.what = { .unit = UnitTypes::Terran_Comsat_Station },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_SCV,
		.what = { .unit = UnitTypes::Terran_Science_Facility },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Academy,
		.what = { .upgrade = UpgradeTypes::U_238_Shells },
		.type = TaskType::UPGRADE,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Starport,
		.what = { .unit = UnitTypes::Terran_Control_Tower },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Starport,
		.what = { .unit = UnitTypes::Terran_Control_Tower },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Science_Facility,
		.what = { .unit = UnitTypes::Terran_Physics_Lab },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_SCV,
		.what = { .unit = UnitTypes::Terran_Armory },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	for (int i = 0; i < 5; i++) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Missile_Turret },
			.type = TaskType::UNIT,
			.where = TilePositions::None,
			.state = TaskState::UNASSIGNED,
		});
	}
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Armory,
		.what = { .upgrade = UpgradeTypes::Terran_Ship_Weapons },
		.type = TaskType::UPGRADE,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	if (ctx.geysers > 1) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Starport },
			.type = TaskType::UNIT,
			.where = TilePositions::None,
			.state = TaskState::UNASSIGNED,
		});
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_Starport,
			.what = { .unit = UnitTypes::Terran_Control_Tower },
			.type = TaskType::UNIT,
			.where = TilePositions::None,
			.state = TaskState::UNASSIGNED,
		});
	}
	for (int i = 0; i < 5; i++) {
		this->build_order.push_back(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = UnitTypes::Terran_Missile_Turret },
			.type = TaskType::UNIT,
			.where = TilePositions::None,
			.state = TaskState::UNASSIGNED,
		});
	}
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_SCV,
		.what = { .unit = UnitTypes::Terran_Bunker },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Armory,
		.what = { .upgrade = UpgradeTypes::Terran_Ship_Weapons },
		.type = TaskType::UPGRADE,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_SCV,
		.what = { .unit = UnitTypes::Terran_Bunker },
		.type = TaskType::UNIT,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Armory,
		.what = { .upgrade = UpgradeTypes::Terran_Ship_Weapons },
		.type = TaskType::UPGRADE,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Armory,
		.what = { .upgrade = UpgradeTypes::Terran_Ship_Plating },
		.type = TaskType::UPGRADE,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Armory,
		.what = { .upgrade = UpgradeTypes::Terran_Ship_Plating },
		.type = TaskType::UPGRADE,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
	this->build_order.push_back(Task {
		.who = UnitTypes::Terran_Armory,
		.what = { .upgrade = UpgradeTypes::Terran_Ship_Plating },
		.type = TaskType::UPGRADE,
		.where = TilePositions::None,
		.state = TaskState::UNASSIGNED,
	});
}

void Build::notify_actor(Task *task)
{
	Actor *actor = this->assigned_build_tasks[task];
	if (actor) actor->on_task_completion();
}

void Build::remove_build_type(BWAPI::UnitType type)
{
	for (auto it = this->build_order.begin(); it != this->build_order.end(); ++it) {
		if (it->type == TaskType::UNIT && it->what.unit == type) {
			this->notify_actor(&(*it));
			this->build_order.erase(it);
			break;
		}
	}
}

void Build::on_unit_complete(Context &ctx, Unit unit)
{
	switch (unit->getType()) {
		case UnitTypes::Terran_Supply_Depot:
			this->building_depot = false;
			break;
		case UnitTypes::Terran_Engineering_Bay:
			// save space
			unit->lift();
			BWEB::Map::onUnitDestroy(unit);
			break;
	}
}

void Build::on_unit_create(Context &ctx, Unit unit)
{
	for (auto it = this->build_order.begin(); it != this->build_order.end(); ++it) {
		if (it->type != TaskType::UNIT) {
			continue;
		}

		Task &task = *it;
		if (task.state == TaskState::COMPLETE
		    && task.what.unit == unit->getType()) {
			this->notify_actor(&(*it));
			this->build_order.erase(it);
			if (task.what.unit.isBuilding()) {
				ctx.reserved_minerals -= task.what.unit.mineralPrice();
				ctx.reserved_gas -= task.what.unit.gasPrice();
			}
			break;
		}
	}
}

void Build::on_unit_morph(Context &ctx, Unit unit)
{
	switch (unit->getType()) {
		case UnitTypes::Terran_Refinery:
			// refinery is not triggered on create event
			ctx.reserved_minerals -= unit->getType().mineralPrice();
			this->remove_build_type(UnitTypes::Terran_Refinery);
			break;
	}
}

void Build::on_unit_destroy(Context &ctx, BWAPI::Unit unit)
{
	Player player = Broodwar->self();
	if (unit->getType().isBuilding() && unit->getPlayer() == player) {
		this->build_order.push_front(Task {
			.who = UnitTypes::Terran_SCV,
			.what = { .unit = unit->getType() },
			.type = TaskType::UNIT,
			.where = unit->getTilePosition(),
			.state = TaskState::UNASSIGNED,
		});
	}
}

void Build::check_supply()
{
	Player player = Broodwar->self();
	if (player->supplyTotal() >= 400) return;

	int scale = 5 + player->supplyUsed() / 20;
	int extra = player->supplyUsed() >= 70;
	if (player->supplyTotal() - player->supplyUsed() <= scale
	    && !this->building_depot) {
		for (int i = 0; i <= extra; i++) {
			this->build_order.push_front(Task {
				.who = UnitTypes::Terran_SCV,
				.what = { .unit = UnitTypes::Terran_Supply_Depot },
				.type = TaskType::UNIT,
				.where = TilePositions::None,
				.state = TaskState::UNASSIGNED,
			});
		}
		this->building_depot = true;
	}
}

static Actor *give_order(Task *task, Context &ctx, map<Unit, Actor *> &actors)
{
	Player player = Broodwar->self();
	for (auto &unit : player->getUnits()) {
		if (unit->getType() != task->who) {
			continue;
		}

		auto actor = actors[unit];
		if (actor && actor->assign_task(ctx, task)) {
			task->assigned_unit = unit;
			return actor;
		}
	}

	return nullptr;
}

bool Build::assign_unit_task(Task *task, Context &ctx, map<Unit, Actor *> &actors)
{
	if (task->what.unit.isBuilding()
	    && task->where == TilePositions::None) {
		task->where = this->find_build_tile(task->what.unit);
	}

	Actor *actor = give_order(task, ctx, actors);

	if (actor && task->what.unit.isBuilding()) {
		ctx.reserved_minerals += task->what.unit.mineralPrice();
		ctx.reserved_gas += task->what.unit.gasPrice();
		this->assigned_build_tasks[task] = actor;

		BWEB::Map::addUsed(task->where, task->what.unit);

		return true;
	}

	return false;
}

bool Build::assign_research_task(Task *task, Context &ctx, map<Unit, Actor *> &actors)
{
	Actor *actor = give_order(task, ctx, actors);
	if (actor && task->what.unit.isBuilding()) {
		return true;
	}

	Unitset units = Broodwar->self()->getUnits();
	for (auto &unit : units) {
		if (unit->getType() == task->who
				&& unit->research(task->what.research)) {
			task->state = TaskState::COMPLETE;
			return true;
		}
	}

	return false;
}

bool Build::assign_upgrade_task(Task *task, Context &ctx, map<Unit, Actor *> &actors)
{
	Actor *actor = give_order(task, ctx, actors);
	if (actor && task->what.unit.isBuilding()) {
		return true;
	}

	Unitset units = Broodwar->self()->getUnits();
	for (auto &unit : units) {
		if (unit->getType() == task->who
				&& unit->upgrade(task->what.upgrade)) {
			task->state = TaskState::COMPLETE;
			return true;
		}
	}

	return false;
}

bool Build::assign_task(Task *task, Context &ctx, map<Unit, Actor *> &actors)
{
	int mineral_price;
	int gas_price;
	switch (task->type) {
		case TaskType::UNIT:
			mineral_price = task->what.unit.mineralPrice();
			gas_price = task->what.unit.gasPrice();
			break;
		case TaskType::RESEARCH:
			mineral_price = task->what.research.mineralPrice();
			gas_price = task->what.research.gasPrice();
			break;
		case TaskType::UPGRADE:
			mineral_price = task->what.upgrade.mineralPrice();
			gas_price = task->what.upgrade.gasPrice();
			break;
	}

	if (ctx.get_minerals() >= mineral_price
	    && ctx.get_gas() >= gas_price) {
		switch (task->type) {
			case TaskType::UNIT:
				return this->assign_unit_task(task, ctx, actors);
			case TaskType::RESEARCH:
				return this->assign_research_task(task, ctx, actors);
			case TaskType::UPGRADE:
				return this->assign_upgrade_task(task, ctx, actors);
		}
	}

	return false;
}

void Build::handle_build_order(Context &ctx, map<Unit, Actor *> &actors)
{
	this->check_supply();

	auto it = this->build_order.begin();
	bool iterate = true;
	while (iterate && it != this->build_order.end()) {
		switch (it->state) {
			case TaskState::UNASSIGNED:
			case TaskState::ADDON_BLOCKED:
				if (this->assign_task(&(*it), ctx, actors)) {
					++it;
				} else if (it->state == TaskState::UNASSIGNED) {
					iterate = false;
				} else if (it->state == TaskState::ADDON_BLOCKED) {
					++it;
				}
				break;
			case TaskState::PENDING_BUILD:
				++it;
				break;
			case TaskState::COMPLETE:
				if (it->type == TaskType::RESEARCH
						|| it->type == TaskType::UPGRADE) {
					it = this->build_order.erase(it);
				} else {
					++it;
				}
				break;
			case TaskState::CANT_BUILD_HERE:
				int width = it->what.unit.tileWidth();
				int height = it->what.unit.tileHeight();
				BWEB::Map::removeUsed(it->where, width, height);
				it->where = this->fix_build_tile(it->assigned_unit, it->what.unit, it->where);
				BWEB::Map::addUsed(it->where, it->what.unit);
				++it;
				break;
		}
	}
}

const list<Task> &Build::get_build_order()
{
	return this->build_order;
}
