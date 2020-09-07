#include <BWAPI.h>
#include <BWAPI/Client.h>

#include <thread>
#include <chrono>

#include "camper_ai.hpp"

using namespace BWAPI;

void reconnect()
{
	while (!BWAPIClient.connect()) {
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
	}
}

void handle_events(CamperAI &ai)
{
	for (auto &e : Broodwar->getEvents()) {
		switch(e.getType()) {
			case EventType::MatchEnd:
				if (e.isWinner())
					Broodwar << "I won the game" << std::endl;
				else
					Broodwar << "I lost the game" << std::endl;
				break;
			case EventType::SendText:
				Broodwar->sendText(e.getText().c_str());
				break;
			case EventType::ReceiveText:
				break;
			case EventType::PlayerLeft:
				break;
			case EventType::NukeDetect:
				if (e.getPosition() != Positions::Unknown) {
					Broodwar->drawCircleMap(e.getPosition(), 40, Colors::Red, true);
					Broodwar << "Nuclear Launch Detected at " << e.getPosition() << std::endl;
				} else {
					Broodwar << "Nuclear Launch Detected" << std::endl;
				}
				break;
			case EventType::UnitComplete:
				ai.on_unit_complete(e.getUnit());
				break;
			case EventType::UnitCreate:
				ai.on_unit_create(e.getUnit());
				break;
			case EventType::UnitDestroy:
				ai.on_unit_destroy(e.getUnit());
				break;
			case EventType::UnitMorph:
				ai.on_unit_morph(e.getUnit());
				break;
			case EventType::UnitDiscover:
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
}

void run()
{
	std::cout << "waiting to enter match" << std::endl;
	while (!Broodwar->isInGame()) {
		BWAPI::BWAPIClient.update();
		if (!BWAPI::BWAPIClient.isConnected()) {
			std::cout << "Reconnecting..." << std::endl;
			reconnect();
		}
	}

	CamperAI ai;
	ai.init();

	while (Broodwar->isInGame()) {
		handle_events(ai);

		ai.update();

		BWAPI::BWAPIClient.update();
		if (!BWAPI::BWAPIClient.isConnected()) {
			std::cout << "Reconnecting..." << std::endl;
			reconnect();
		}
	}

	std::cout << "Game ended" << std::endl;
}

int main(int argc, const char *argv[])
{
	std::cout << "Connecting..." << std::endl;
	reconnect();
	while (true) {
		run();
	}

	std::cout << "Press ENTER to continue..." << std::endl;
	std::cin.ignore();

	return 0;
}
