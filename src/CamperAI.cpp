#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <BWTA.h>

#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <thread>

using namespace std;
using namespace BWAPI;

struct Build {
	UnitType who;
	UnitType what;
	bool pending;
};

struct CamperAI {
	Unit base;
	Unitset mineral_fields;
	int reserved_minerals;

	bool building_depot;
	list<Build> build_order;

	map<Unit, pair<UnitType, TilePosition>> worker_tasks;

	BWTA::Region *region;

	int get_minerals() const {
		return Broodwar->self()->minerals() - reserved_minerals;
	}
};

void draw_terrain_data();
void draw_build_order(list<Build> &build_order);
void draw_stats();
void drawBullets();
void drawVisibilityData();
void showPlayers();
void showForces();
bool show_bullets;
bool show_visibility_data;
TilePosition find_build_tile(CamperAI &ai, UnitType unit_type);

Unit worker = NULL;

void reconnect()
{
	while (!BWAPIClient.connect()) {
		this_thread::sleep_for(chrono::milliseconds{ 1000 });
	}
}

Unit pick_random_unit(Unitset &units)
{
	auto it = units.begin();
	advance(it, rand() % units.size());
	return *it;
}

void handle_cc(Unit cc, CamperAI &ai)
{
	if (!cc->isTraining() && ai.get_minerals() >= 50) {
		cc->train(UnitTypes::Terran_SCV);
	}
}

void handle_scv(Unit scv, CamperAI &ai)
{
	if (!scv->isCompleted()) return;

	if (scv->isIdle()) {
		auto it = ai.worker_tasks.find(scv);
		if (it == ai.worker_tasks.end()) {
			scv->gather(pick_random_unit(ai.mineral_fields));
		} else {
			UnitType what = it->second.first;
			TilePosition where = it->second.second;
			Broodwar << "Building" << endl;
			if (!scv->build(what, where)) {
				Broodwar << "build failed" << endl;
			}
			ai.worker_tasks.erase(it);
		}
		return;
	}

	if (!scv->isGatheringMinerals()) return;

	// TODO do not ignore `who` in `Build`
	if (!ai.build_order.empty()) {
		Build &build = ai.build_order.front();
		if (!build.pending && build.what.isBuilding()
		    && ai.get_minerals() >= build.what.mineralPrice()) {
			TilePosition pos = find_build_tile(ai, build.what);

			if (!Broodwar->isVisible(pos)) {
				Broodwar << "Moving" << endl;
				scv->move(Position(pos));
				ai.worker_tasks[scv] = { build.what, pos };
			} else {
				Broodwar << "Building" << endl;
				if (!scv->build(build.what, pos)) {
					Broodwar << "build failed" << endl;
				}
			}
			build.pending = true;
			ai.reserved_minerals = build.what.mineralPrice();
			worker = scv;
		}
	}
}

void check_supply(CamperAI &ai)
{
	Player player = Broodwar->self();
	if (player->supplyTotal() - player->supplyUsed() <= 6
	    && !ai.building_depot) {
		ai.build_order.push_front(Build {
			.who = UnitTypes::Terran_SCV,
			.what = UnitTypes::Terran_Supply_Depot,
		});
		ai.building_depot = true;
	}
}

bool is_forbidden_area(CamperAI &ai, TilePosition target)
{
	const int max_dist = 90;

	Position pos = Position(target);
	for (auto &mineral : ai.mineral_fields) {
		if (mineral->getDistance(pos) <= max_dist)
			return true;
	}

	return ai.base->getPosition().getDistance(pos) <= max_dist;
}

bool is_choke_too_close(BWTA::Chokepoint *choke, TilePosition target)
{
	Position pos = Position(target);
	if (choke->getCenter().getDistance(pos) < 90)
		return true;

	return false;
}

TilePosition find_geyser_tile(CamperAI &ai)
{
	TilePosition center = TilePosition(ai.region->getCenter());
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

TilePosition find_build_tile(CamperAI &ai, UnitType unit_type)
{
	if (unit_type == UnitTypes::Terran_Refinery)
		return find_geyser_tile(ai);

	TilePosition pos = TilePosition(ai.region->getCenter());
	BWTA::Chokepoint *choke = BWTA::getNearestChokepoint(pos);

	int iters = 0;
	while (!Broodwar->canBuildHere(pos, unit_type)
	       || ai.region != BWTA::getRegion(pos)
	       || is_forbidden_area(ai, pos)
	       || is_choke_too_close(choke, pos)) {

		pos = TilePosition(ai.region->getCenter());
		pos.x += rand() % 80 - 40;
		pos.y += rand() % 80 - 40;

		iters++;
		if (iters == 100) {
			Broodwar << "suitable build position not found" << endl;
			break;
		}
	}

	return pos;
}

void handle_unit_complete(CamperAI &ai, Unit unit)
{
	Broodwar << unit->getType() << " completed" << endl;
	if (unit->getType() == UnitTypes::Terran_Supply_Depot) {
		ai.building_depot = false;
	} else if (unit->getType() == UnitTypes::Terran_Refinery) {
		// refinery is not triggered on create event
		for (auto it = ai.build_order.begin(); it != ai.build_order.end(); ++it) {
			if (it->what == UnitTypes::Terran_Refinery) {
				ai.build_order.erase(it);
				break;
			}
		}
		ai.reserved_minerals -= unit->getType().mineralPrice();
	}

	if (unit->getType().isRefinery()) {
		Unitset close_workers = unit->getUnitsInRadius(500, Filter::IsWorker);
		int total_workers = 1;
		for (auto &scv : close_workers) {
			if (!scv->isGatheringMinerals())
				continue;

			scv->gather(unit);

			total_workers++;
			if (total_workers == 3)
				break;
		}
	}
}

void handle_unit_create(CamperAI &ai, Unit unit)
{
	Broodwar << unit->getType() << " created" << endl;
	for (auto it = ai.build_order.begin(); it != ai.build_order.end(); ++it) {
		Build &build = *it;
		if (!build.pending) break;
		if (build.what == unit->getType()) {
			ai.build_order.erase(it);
			ai.reserved_minerals -= build.what.mineralPrice();
			break;
		}
	}
}

void ai_tick(CamperAI &ai)
{
	draw_terrain_data();

	BWTA::Chokepoint *choke = BWTA::getNearestChokepoint(ai.base->getPosition());
	Broodwar->drawCircleMap(choke->getCenter(), 90, Colors::Green);
	Broodwar->drawCircleMap(ai.region->getCenter(), 20, Colors::Green);

	if (worker != NULL) {
		Broodwar->drawCircleMap(worker->getPosition(), 9, Colors::Red);
	}

	check_supply(ai);

	for (auto &unit : Broodwar->self()->getUnits()) {
		switch (unit->getType()) {
			case UnitTypes::Terran_Command_Center:
				handle_cc(unit, ai);
				break;
			case UnitTypes::Terran_SCV:
				handle_scv(unit, ai);
				break;
			default:
				break;
		}
	}

	for (auto &e : Broodwar->getEvents()) {
		switch(e.getType()) {
			case EventType::MatchEnd:
				if (e.isWinner())
					Broodwar << "I won the game" << endl;
				else
					Broodwar << "I lost the game" << endl;
				break;
			case EventType::SendText:
				if (e.getText()=="/show bullets") {
					show_bullets=!show_bullets;
				} else if (e.getText()=="/show players") {
					showPlayers();
				} else if (e.getText()=="/show forces") {
					showForces();
				} else if (e.getText()=="/show visibility") {
					show_visibility_data=!show_visibility_data;
				} else {
					Broodwar->sendText(e.getText().c_str());
				}
				break;
			case EventType::ReceiveText:
				break;
			case EventType::PlayerLeft:
				break;
			case EventType::NukeDetect:
				if (e.getPosition() != Positions::Unknown) {
					Broodwar->drawCircleMap(e.getPosition(), 40, Colors::Red, true);
					Broodwar << "Nuclear Launch Detected at " << e.getPosition() << endl;
				} else {
					Broodwar << "Nuclear Launch Detected" << endl;
				}
				break;
			case EventType::UnitComplete:
				handle_unit_complete(ai, e.getUnit());
				break;
			case EventType::UnitCreate:
				handle_unit_create(ai, e.getUnit());
				break;
			case EventType::UnitDestroy:
				break;
			case EventType::UnitMorph:
				break;
			case EventType::UnitShow:
				break;
			case EventType::UnitHide:
				break;
			case EventType::UnitRenegade:
				break;
			case EventType::SaveGame:
				Broodwar->sendText("The game was saved to \"%s\".", e.getText().c_str());
				break;
			default:
				break;
		}
	}

	if (show_bullets)
		drawBullets();

	if (show_visibility_data)
		drawVisibilityData();

	draw_build_order(ai.build_order);

	draw_stats();
	Broodwar->drawTextScreen(300, 0, "APM: %d",Broodwar->getAPM());

	BWAPI::BWAPIClient.update();
	if (!BWAPI::BWAPIClient.isConnected()) {
		cout << "Reconnecting..." << endl;
		reconnect();
	}
}

void ai_run()
{
	cout << "waiting to enter match" << endl;
	while (!Broodwar->isInGame()) {
		BWAPI::BWAPIClient.update();
		if (!BWAPI::BWAPIClient.isConnected())
		{
			cout << "Reconnecting..." << endl;;
			reconnect();
		}
	}

	cout << "starting match!" << endl;
	Broodwar->sendText("Hello world!");
	Broodwar << "The map is " << Broodwar->mapName() << ", a " << Broodwar->getStartLocations().size() << " player map" << endl;
	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	// Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	Broodwar->setLocalSpeed(20);

	BWTA::analyze();

	srand(chrono::steady_clock::now().time_since_epoch().count());

	show_bullets=false;
	show_visibility_data=false;

	TilePosition start_tile = Broodwar->self()->getStartLocation();
	Position start_pos = Position(start_tile.x * 32, start_tile.y * 32);

	CamperAI ai;
	ai.reserved_minerals = 0;
	ai.base = Broodwar->getClosestUnit(start_pos, Filter::IsResourceDepot);
	ai.mineral_fields = ai.base->getUnitsInRadius(256, Filter::IsMineralField);

	ai.build_order.push_back(Build {
		.who = UnitTypes::Terran_SCV,
		.what = UnitTypes::Terran_Barracks,
	});
	ai.build_order.push_back(Build {
		.who = UnitTypes::Terran_SCV,
		.what = UnitTypes::Terran_Refinery,
	});
	ai.build_order.push_back(Build {
		.who = UnitTypes::Terran_SCV,
		.what = UnitTypes::Terran_Bunker,
	});

	Broodwar << "Mineral fields: " << ai.mineral_fields.size() << endl;

	ai.region = BWTA::getRegion(ai.base->getPosition());

	while (Broodwar->isInGame()) {
		ai_tick(ai);
	}

	cout << "Game ended" << endl;
}

int main(int argc, const char* argv[])
{
	cout << "Connecting..." << endl;;
	reconnect();
	while (true) {
		ai_run();
	}
	cout << "Press ENTER to continue..." << endl;
	cin.ignore();
	return 0;
}

void draw_build_order(list<Build> &build_order)
{
	int line = 1;
	Broodwar->drawTextScreen(5, 0, "Build order:");
	for (auto &build : build_order) {
		Broodwar->drawTextScreen(5, 16 * line, "%s: %s, pending: %s",
				build.who.c_str(),
				build.what.c_str(),
				build.pending ? "true" : "false");
		line++;
		if (line > 4) {
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

void draw_terrain_data()
{
	//we will iterate through all the base locations, and draw their outlines.
	for (const auto& baseLocation : BWTA::getBaseLocations()) {
		TilePosition p = baseLocation->getTilePosition();

		//draw outline of center location
		Position leftTop(p.x * TILE_SIZE, p.y * TILE_SIZE);
		Position rightBottom(leftTop.x + 4 * TILE_SIZE, leftTop.y + 3 * TILE_SIZE);
		Broodwar->drawBoxMap(leftTop, rightBottom, Colors::Blue);

		//draw a circle at each mineral patch
		for (const auto& mineral : baseLocation->getStaticMinerals()) {
			Broodwar->drawCircleMap(mineral->getInitialPosition(), 20, Colors::Cyan);
		}

		//draw the outlines of Vespene geysers
		for (const auto& geyser : baseLocation->getGeysers()) {
			TilePosition p1 = geyser->getInitialTilePosition();
			Position leftTop1(p1.x * TILE_SIZE, p1.y * TILE_SIZE);
			Position rightBottom1(leftTop1.x + 4 * TILE_SIZE, leftTop1.y + 2 * TILE_SIZE);
			Broodwar->drawBoxMap(leftTop1, rightBottom1, Colors::Orange);
		}

		//if this is an island expansion, draw a yellow circle around the base location
		if (baseLocation->isIsland()) {
			Broodwar->drawCircleMap(baseLocation->getPosition(), 80, Colors::Yellow);
		}
	}

	//we will iterate through all the regions and ...
	for (const auto& region : BWTA::getRegions()) {
		// draw the polygon outline of it in green
		const BWTA::Polygon *p = &region->getPolygon();
		for (size_t j = 0; j < p->size(); ++j) {
			Position point1 = (*p)[j];
			Position point2 = (*p)[(j + 1) % p->size()];
			Broodwar->drawLineMap(point1, point2, Colors::Green);
		}
		// visualize the chokepoints with red lines
		for (auto const& chokepoint : region->getChokepoints()) {
			Position point1 = chokepoint->getSides().first;
			Position point2 = chokepoint->getSides().second;
			Broodwar->drawLineMap(point1, point2, Colors::Red);
		}
	}

}
