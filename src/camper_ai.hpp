#ifndef __CAMPER_AI_HPP__
#define __CAMPER_AI_HPP__

#include <set>

#include <BWAPI.h>

#include "actor.hpp"
#include "army.hpp"
#include "build.hpp"
#include "common.hpp"

using namespace BWAPI;

class CamperAI {
	std::map<BWAPI::Unit, Actor *> actors;
	Build build;
	Context ctx;
	Army army;
	void draw_ui();
public:
	CamperAI();
	~CamperAI();
	void init();
	void update();
	void on_unit_complete(Unit unit);
	void on_unit_create(Unit unit);
	void on_unit_morph(Unit unit);
	void on_unit_destroy(Unit unit);
};


#endif
