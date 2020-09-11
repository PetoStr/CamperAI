#include "army.hpp"

#include <BWEB.h>

#include "battlecruiser.hpp"

using namespace BWAPI;
using namespace std;

void Army::init()
{
	this->last_order = ArmyOrder {
		.type = Orders::Guard,
		.where = this->get_guard_pos(),
	};
}

void Army::order_all()
{
	for (auto &it : this->units) {
		it.second->receive_order(this->last_order);
	}
}

void Army::handle_guard()
{
	if (this->units.size() >= 8 && this->some_units_in_pos()) {
		Broodwar << "time to attack" << endl;
		this->last_order.type = Orders::AttackMove;
		this->last_order.where = this->get_attack_pos();
		this->order_all();
	}
}

void Army::handle_attack()
{
	if (this->units.size() < 5) {
		this->last_order.type = Orders::Guard;
		this->last_order.where = this->get_guard_pos();
		this->order_all();
	} else if (this->some_units_in_pos()) {
		this->last_order.where = this->get_attack_pos();
		this->order_all();
	}
}

void Army::update()
{
	this->remember_enemy();
	/*Broodwar->drawTextScreen(300, 32, "Enemy buildings: %d",
			this->enemy_buildings.size());*/

	switch (this->last_order.type) {
		case Orders::Guard:
			this->handle_guard();
			break;
		case Orders::AttackMove:
			this->handle_attack();
			break;
		default:
			break;
	}
}

void Army::on_unit_complete(Unit unit, ArmyUnit *army_unit)
{
	army_unit->receive_order(this->last_order);
	this->units[unit] = army_unit;
}

void Army::on_unit_destroy(BWAPI::Unit unit)
{
	this->units.erase(unit);
}

Position Army::get_guard_pos()
{
	return Position(BWEB::Map::getMainChoke()->Center());
}

Position get_possible_enemy_loc()
{
	TilePosition base = Broodwar->self()->getStartLocation();
	TilePosition best = TilePositions::None;
	double best_dist = 2e9;
	for (TilePosition pos : Broodwar->getStartLocations()) {
		if (pos != base && !Broodwar->isExplored(pos)) {
			double dist = base.getDistance(pos);
			if (dist < best_dist) {
				best = pos;
				best_dist = dist;
			}
		}
	}

	if (best != TilePositions::None) {
		return Position(best);
	} else {
		return BWEM::Map::Instance().RandomPosition();
	}
}

Position Army::get_attack_pos()
{
	if (this->enemy_buildings.empty()) {
		return get_possible_enemy_loc();
	}

	Position best = Positions::None;
	double best_dist = 2e9;
	for (Position pos : this->enemy_buildings) {
		double dist = this->last_order.where.getDistance(pos);
		if (dist < best_dist) {
			best_dist = dist;
			best = pos;
		}
	}

	return best;
}

bool Army::some_units_in_pos()
{
	int cn = 0;
	for (auto &it : this->units) {
		int dist = it.first->getDistance(this->last_order.where);
		if (dist < 100) {
			cn++;
		}
	}

	return cn >= 5;
}

void Army::remember_enemy()
{
	for (auto &enemy : Broodwar->enemies()) {
		for (auto &unit : enemy->getUnits()) {
			if (unit->getType().isBuilding()) {
				this->enemy_buildings.insert(unit->getPosition());
			}
		}
	}

	auto it = this->enemy_buildings.begin();
	while (it != this->enemy_buildings.end()) {
		if (!Broodwar->isVisible(TilePosition(*it))) {
			++it;
			continue;
		}

		bool ok = false;
		for (auto &enemy : Broodwar->enemies()) {
			for (auto &unit : enemy->getUnits()) {
				if (!unit->getType().isBuilding()) continue;
				if (unit->getPosition() == *it) {
					ok = true;
					break;
				}
			}
		}

		if (ok) {
			++it;
		} else {
			it = this->enemy_buildings.erase(it);
		}
	}
}
