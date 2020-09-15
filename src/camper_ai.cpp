#include <BWAPI.h>

#include <bwem.h>
#include <BWEB.h>

#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "barracks.hpp"
#include "battlecruiser.hpp"
#include "camper_ai.hpp"
#include "command_center.hpp"
#include "comsat.hpp"
#include "factory.hpp"
#include "marine.hpp"
#include "science_facility.hpp"
#include "scv.hpp"
#include "starport.hpp"
#include "tank.hpp"

using namespace std;
using namespace BWAPI;

static auto &game_map = BWEM::Map::Instance();

static void send_workers_to_refinery(Unit refinery)
{
	Unitset close_workers = refinery->getUnitsInRadius(500, Filter::IsWorker);
	int total_workers = 1;
	for (auto &scv : close_workers) {
		if (!scv->isGatheringMinerals())
			continue;

		scv->gather(refinery);

		total_workers++;
		if (total_workers == 3)
			break;
	}
}

static void draw_build_order(const list<Task> &build_order)
{
	int line = 1;
	Broodwar->drawTextScreen(5, 0, "Build order:");
	for (auto &task : build_order) {
		const char *what;
		switch (task.type) {
			case TaskType::UNIT:
				what = task.what.unit.c_str();
				break;
			case TaskType::RESEARCH:
				what = task.what.research.c_str();
				break;
			case TaskType::UPGRADE:
				what = task.what.upgrade.c_str();
				break;
			case TaskType::NONE:
				what = "Unknown";
				break;
		}

		Broodwar->drawTextScreen(5, 16 * line, "%s: %s (%s)",
				task.who.c_str(),
				what,
				task_state_str(task.state));
		line++;
		if (line > 5) {
			break;
		}
	}
}

void draw_game_map()
{
	try {
		BWEM::utils::gridMapExample(game_map);
		BWEM::utils::drawMap(game_map);
	} catch (const exception &e) {
		Broodwar << "Exception: " << e.what() << endl;
	}
	BWEB::Map::draw();
}

CamperAI::CamperAI()
{
	this->ctx = Context { 0 };
}

void CamperAI::on_unit_complete(Unit unit)
{
	// we do not handle enemy units yet
	if (unit->getPlayer() != Broodwar->self()) {
		return;
	}

	this->build.on_unit_complete(this->ctx, unit);

	switch (unit->getType()) {
		case UnitTypes::Terran_Supply_Depot:
			break;
		case UnitTypes::Terran_Refinery:
			send_workers_to_refinery(unit);
			break;
		case UnitTypes::Terran_Marine:
			this->actors[unit] = make_shared<Marine>(unit);
			break;
		case UnitTypes::Terran_Siege_Tank_Tank_Mode:
		case UnitTypes::Terran_Siege_Tank_Siege_Mode:
			this->actors[unit] = make_shared<Tank>(unit);
			break;
		case UnitTypes::Terran_SCV:
			this->actors[unit] = make_shared<SCV>(unit);
			break;
		case UnitTypes::Terran_Command_Center:
			this->actors[unit] = make_shared<CommandCenter>(unit);
			break;
		case UnitTypes::Terran_Barracks:
			this->actors[unit] = make_shared<Barracks>(unit);
			break;
		case UnitTypes::Terran_Factory:
			this->actors[unit] = make_shared<Factory>(unit);
			break;
		case UnitTypes::Terran_Starport:
			this->actors[unit] = make_shared<Starport>(unit);
			break;
		case UnitTypes::Terran_Science_Facility:
			this->actors[unit] = make_shared<ScienceFacility>(unit);
			break;
		case UnitTypes::Terran_Comsat_Station:
			this->actors[unit] = make_shared<Comsat>(unit);
			break;
		case UnitTypes::Terran_Battlecruiser:
			auto bc = make_shared<Battlecruiser>(unit);
			this->actors[unit] = bc;
			this->army.on_unit_complete(unit, bc);
			break;
	}
}

void CamperAI::on_unit_create(Unit unit)
{
	//Broodwar << unit->getType() << " created" << endl;
	BWEB::Map::onUnitDiscover(unit);

	if (unit->getPlayer() == Broodwar->self()) {
		this->build.on_unit_create(this->ctx, unit);
	}
}

void CamperAI::on_unit_morph(Unit unit)
{
	BWEB::Map::onUnitMorph(unit);

	if (unit->getPlayer() == Broodwar->self()) {
		this->build.on_unit_morph(this->ctx, unit);
	}
}

void CamperAI::on_unit_destroy(Unit unit)
{
	this->build.on_unit_destroy(this->ctx, unit);
	this->army.on_unit_destroy(unit);

	BWEB::Map::onUnitDestroy(unit);

	auto &actor = this->actors[unit];
	if (actor) {
		actor->on_destroy();
	}

	try {
		if (unit->getType().isMineralField())
			game_map.OnMineralDestroyed(unit);
		else if (unit->getType().isSpecialBuilding())
			game_map.OnStaticBuildingDestroyed(unit);
	} catch (const exception &e) {
		Broodwar << "Exception: " << e.what() << endl;
	}
}

void CamperAI::draw_ui()
{
	//draw_game_map();

	draw_build_order(this->build.get_build_order());

	Broodwar->drawTextScreen(300, 0, "APM: %d", Broodwar->getAPM());
	Broodwar->drawTextScreen(300, 16, "FRAME: %d", Broodwar->getFrameCount());
}

void CamperAI::update()
{
	this->draw_ui();

	if (Broodwar->getFrameCount() % Broodwar->getLatency() != 0) {
		return;
	}

	this->build.handle_build_order(this->ctx, this->actors);
	this->army.update();

	for (auto &unit : Broodwar->self()->getUnits()) {
		if (!unit->exists()) continue;
		auto actor = this->actors[unit];
		if (actor) {
			actor->act(this->ctx);
		}
	}
}

static int calc_geyser_cn()
{
	TilePosition center = BWEB::Map::getMainTile();
	Unitset geysers = Broodwar->getGeysers();
	int cn = 0;
	for (auto &geyser : geysers) {
		double dist = center.getDistance(geyser->getTilePosition());
		if (dist < 16) {
			cn++;
		}
	}

	return cn;
}

void CamperAI::init()
{

	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	// Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	//Broodwar->setLocalSpeed(20);
	Broodwar->setLocalSpeed(0);

	try {
		game_map.Initialize();
		game_map.EnableAutomaticPathAnalysis();

		BWEM::utils::MapPrinter::Initialize(&game_map);
		BWEM::utils::printMap(game_map);
		BWEM::utils::pathExample(game_map);
	} catch (const exception &e) {
		Broodwar << "Exception: " << e.what() << endl;
	}
	BWEB::Map::onStart();
	this->build.create_wall();
	BWEB::Blocks::findBlocks();
	BWEB::Stations::findStations();

	srand(chrono::steady_clock::now().time_since_epoch().count());

	this->army.init();

	TilePosition start_tile = Broodwar->self()->getStartLocation();
	Position start_pos = Position(start_tile.x * 32, start_tile.y * 32);

	this->ctx.reserved_minerals = 0;
	this->ctx.base = Broodwar->getClosestUnit(start_pos, Filter::IsResourceDepot);
	this->ctx.mineral_fields = this->ctx.base->getUnitsInRadius(256, Filter::IsMineralField);
	this->ctx.geysers = calc_geyser_cn();

	this->build.create_build_order(this->ctx);
}
