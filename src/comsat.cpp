#include "comsat.hpp"

using namespace BWAPI;

const int WAIT_FRAMES = 30;

Comsat::Comsat(Unit _unit) : unit(_unit)
{
	this->wait_frames = 0;
}

void Comsat::scan()
{
	Unit comsat = this->unit;

	for (const Player &enemy : Broodwar->enemies()) {
		for (const Unit &unit : enemy->getUnits()) {
			if (unit->isVisible()
			    && !unit->isDetected()
			    && this->is_ally_close(unit)) {
				comsat->useTech(TechTypes::Scanner_Sweep,
						unit->getPosition());
				this->wait_frames = WAIT_FRAMES;
				return;
			}
		}
	}
}

void Comsat::act(Context &ctx)
{
	if (this->wait_frames != 0) {
		this->wait_frames--;
		return;
	}

	Unit comsat = this->unit;
	if (comsat->getEnergy() >= TechTypes::Scanner_Sweep.energyCost()) {
		this->scan();
	}
}

bool Comsat::is_ally_close(BWAPI::Unit enemy)
{
	Player player = Broodwar->self();

	for (const Unit &unit : player->getUnits()) {
		if (unit->isInWeaponRange(enemy)) {
			return true;
		}
	}

	return false;
}
