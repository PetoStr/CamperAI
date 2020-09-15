#include "battlecruiser.hpp"

#include <cstring>

using namespace BWAPI;

const int GUARD_RANGE = 300;

Battlecruiser::Battlecruiser(Unit _unit) : unit(_unit)
{
	this->order = ArmyOrder {
		.type = Orders::Nothing,
	};
	this->new_order = false;
}

void Battlecruiser::act(Context &ctx)
{
	Unit bc = this->unit;
	if (this->order.type == Orders::Nothing) return;

	switch (this->order.type) {
		case Orders::AttackMove:
			if (this->new_order) {
				bc->attack(this->order.where);
				this->new_order = false;
			}
			break;
		case Orders::Guard: {
			double order_dist = bc->getPosition()
				.getDistance(this->order.where);
			if (order_dist > GUARD_RANGE) {
				bc->attack(this->order.where);
			}
			break;
		}
		default:
			break;
	}
}

void Battlecruiser::receive_order(ArmyOrder &order)
{
	memcpy(&this->order, &order, sizeof(order));
	this->new_order = true;
}
