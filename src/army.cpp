#include "army.hpp"

#include <cfloat>

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
	if (this->units.size() >= 5 && this->some_units_in_pos()) {
		this->last_order.type = Orders::AttackMove;
		this->last_order.where = this->get_attack_pos();
		this->order_all();
	}
}

void Army::handle_attack()
{
	if (this->units.size() <= 2) {
		this->last_order.type = Orders::Guard;
		this->last_order.where = this->get_guard_pos();
		this->order_all();
	} else if (this->some_units_in_pos()) {
		Position attack_pos = this->get_attack_pos();
		if (attack_pos != this->last_order.where) {
			this->last_order.where = attack_pos;
			this->order_all();
		}
	}
}

void Army::update()
{
	this->remember_enemy();

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
	// update guard pos at least when new army unit is created
	if (this->last_order.type == Orders::Guard) {
		this->last_order.where = this->get_guard_pos();
	}

	army_unit->receive_order(this->last_order);
	this->units[unit] = army_unit;
}

void Army::on_unit_destroy(BWAPI::Unit unit)
{
	this->units.erase(unit);
}

Position Army::get_guard_pos()
{

	Position choke = Position(BWEB::Map::getMainChoke()->Center());
	double best_dist = DBL_MAX;
	Position best = Positions::Invalid;
	for (auto &unit : Broodwar->self()->getUnits()) {
		if (unit->getType() == UnitTypes::Terran_Bunker) {
			double dist = unit->getDistance(choke);
			if (dist < best_dist) {
				best_dist = dist;
				best = unit->getPosition();
			}
		}
	}

	if (best.isValid()) {
		return best;
	} else {
		return choke;
	}
}

Position Army::get_possible_enemy_loc()
{
	TilePosition base = Broodwar->self()->getStartLocation();
	TilePosition here = TilePosition(this->last_order.where);
	TilePosition best = TilePositions::Invalid;
	double best_dist = DBL_MAX;
	for (TilePosition pos : Broodwar->getStartLocations()) {
		if (pos != base && !Broodwar->isExplored(pos)) {
			double dist = here.getDistance(pos);
			if (dist < best_dist) {
				best = pos;
				best_dist = dist;
			}
		}
	}

	if (best.isValid()) {
		return Position(best);
	} else {
		return BWEM::Map::Instance().RandomPosition();
	}
}

Position Army::get_attack_pos()
{
	if (this->enemy_buildings.empty()) {
		return this->get_possible_enemy_loc();
	}

	Position best = Positions::Invalid;
	double best_dist = DBL_MAX;
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
		if (dist < 200) {
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
