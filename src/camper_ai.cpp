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

namespace { auto &game_map = BWEM::Map::Instance(); }

bool show_bullets;
bool show_visibility_data;

void send_workers_to_refinery(Unit refinery)
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

void draw_build_order(const list<Task> &build_order)
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

void draw_stats()
{
	int line = 1;
	Broodwar->drawTextScreen(5, 96, "I have %d units:", Broodwar->self()->allUnitCount() );
	for (auto& unitType : UnitTypes::allUnitTypes()) {
		int count = Broodwar->self()->allUnitCount(unitType);
		if (count) {
			Broodwar->drawTextScreen(5, 96 + 16*line, "- %d %s%c", count, unitType.c_str(), count == 1 ? ' ' : 's');
			++line;
		}
	}
}

void drawBullets()
{
	for (auto &b : Broodwar->getBullets())
	{
		Position p = b->getPosition();
		double velocityX = b->getVelocityX();
		double velocityY = b->getVelocityY();
		Broodwar->drawLineMap(p, p + Position((int)velocityX, (int)velocityY), b->getPlayer() == Broodwar->self() ? Colors::Green : Colors::Red);
		Broodwar->drawTextMap(p, "%c%s", b->getPlayer() == Broodwar->self() ? Text::Green : Text::Red, b->getType().c_str());
	}
}

void drawVisibilityData()
{
	int wid = Broodwar->mapHeight(), hgt = Broodwar->mapWidth();
	for ( int x = 0; x < wid; ++x )
		for ( int y = 0; y < hgt; ++y )
		{
			if ( Broodwar->isExplored(x, y) )
				Broodwar->drawDotMap(x*32+16, y*32+16, Broodwar->isVisible(x, y) ? Colors::Green : Colors::Blue);
			else
				Broodwar->drawDotMap(x*32+16, y*32+16, Colors::Red);
		}
}

void showPlayers()
{
	Playerset players = Broodwar->getPlayers();
	for(auto p : players)
		Broodwar << "Player [" << p->getID() << "]: " << p->getName() << " is in force: " << p->getForce()->getName() << endl;
}

void showForces()
{
	Forceset forces=Broodwar->getForces();
	for(auto f : forces)
	{
		Playerset players = f->getPlayers();
		Broodwar << "Force " << f->getName() << " has the following players:" << endl;
		for(auto p : players)
			Broodwar << "  - Player [" << p->getID() << "]: " << p->getName() << endl;
	}
}

void draw_game_map()
{
	/*try {
		BWEM::utils::gridMapExample(game_map);
		BWEM::utils::drawMap(game_map);
	} catch (const exception &e) {
		Broodwar << "Exception: " << e.what() << endl;
	}*/
	BWEB::Map::draw();
}

CamperAI::CamperAI()
{
	this->ctx = Context { 0 };
}

CamperAI::~CamperAI()
{
	for (auto p : this->actors) {
		delete p.second;
	}
}

void CamperAI::on_unit_complete(Unit unit)
{
	// we do not handle enemies yet
	if (unit->getPlayer() != Broodwar->self()) {
		return;
	}

	this->build.on_unit_complete(this->ctx, unit);

	//Broodwar << unit->getType() << " completed" << endl;
	switch (unit->getType()) {
		case UnitTypes::Terran_Supply_Depot:
			break;
		case UnitTypes::Terran_Refinery:
			send_workers_to_refinery(unit);
			break;
		case UnitTypes::Terran_Marine:
			this->actors[unit] = new Marine(unit);
			break;
		case UnitTypes::Terran_Siege_Tank_Tank_Mode:
		case UnitTypes::Terran_Siege_Tank_Siege_Mode:
			this->actors[unit] = new Tank(unit);
			break;
		case UnitTypes::Terran_SCV:
			this->actors[unit] = new SCV(unit);
			break;
		case UnitTypes::Terran_Command_Center:
			this->actors[unit] = new CommandCenter(unit);
			break;
		case UnitTypes::Terran_Barracks:
			this->actors[unit] = new Barracks(unit);
			break;
		case UnitTypes::Terran_Factory:
			this->actors[unit] = new Factory(unit);
			break;
		case UnitTypes::Terran_Starport:
			this->actors[unit] = new Starport(unit);
			break;
		case UnitTypes::Terran_Science_Facility:
			this->actors[unit] = new ScienceFacility(unit);
			break;
		case UnitTypes::Terran_Comsat_Station:
			this->actors[unit] = new Comsat(unit);
			break;
		case UnitTypes::Terran_Battlecruiser:
			ArmyUnit *bc = new Battlecruiser(unit);
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

	Actor *actor = this->actors[unit];
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
	draw_game_map();

	if (show_bullets)
		drawBullets();

	if (show_visibility_data)
		drawVisibilityData();

	draw_build_order(this->build.get_build_order());

	//draw_stats();
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

	cout << "starting match!" << endl;
	Broodwar << "The map is " << Broodwar->mapName() << ", a " << Broodwar->getStartLocations().size() << " player map" << endl;
	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	// Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	//Broodwar->setLocalSpeed(20);
	Broodwar->setLocalSpeed(0);

	try {
		Broodwar << "Map init..." << endl;

		game_map.Initialize();
		game_map.EnableAutomaticPathAnalysis();

		BWEM::utils::MapPrinter::Initialize(&game_map);
		BWEM::utils::printMap(game_map);
		BWEM::utils::pathExample(game_map);

		Broodwar << "Map init finished." << endl;
	} catch (const exception &e) {
		Broodwar << "Exception: " << e.what() << endl;
	}
	BWEB::Map::onStart();
	this->build.create_wall();
	BWEB::Blocks::findBlocks();
	BWEB::Stations::findStations();

	Broodwar << "Walls: " << BWEB::Walls::getWalls().size() << endl;

	srand(chrono::steady_clock::now().time_since_epoch().count());

	this->army.init();

	show_bullets = false;
	show_visibility_data = false;

	TilePosition start_tile = Broodwar->self()->getStartLocation();
	Position start_pos = Position(start_tile.x * 32, start_tile.y * 32);

	this->ctx.reserved_minerals = 0;
	this->ctx.base = Broodwar->getClosestUnit(start_pos, Filter::IsResourceDepot);
	this->ctx.mineral_fields = this->ctx.base->getUnitsInRadius(256, Filter::IsMineralField);
	this->ctx.geysers = calc_geyser_cn();

	this->build.create_build_order(this->ctx);

	Broodwar << "Mineral fields: " << this->ctx.mineral_fields.size() << endl;
	Broodwar << "Geysers: " << this->ctx.geysers << endl;
}
