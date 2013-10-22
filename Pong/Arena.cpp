#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_ARENA
#include "Arena.h"
#endif
#include "PongGame.h"
using namespace Pong;
// ------------------------------------ //
Pong::Arena::Arena(shared_ptr<Leviathan::GameWorld> world) : TargetWorld(world){

}

Pong::Arena::~Arena(){

}
// ------------------------------------ //
bool Pong::Arena::GenerateArena(PongGame* game, vector<PlayerSlot*> &players, int plycount, int maximumsplit, bool clearfirst /*= true*/){
	// check sanity of values //
	QUICKTIME_THISSCOPE;

	if(plycount == 0 || plycount > 4){
		game->SetError("Player count must be between 1 and 4");
		return false;
	}

	if(maximumsplit > 2){
		game->SetError("Sides have to be split into two or be whole (max 2 players per side)");
		return false;
	}

	if(clearfirst){
		TargetWorld->ClearObjects();
		_ClearPointers();
	}

	// calculate sizes //
	float width = 20*BASE_ARENASCALE;
	float height = width;

	float mheight = 3*BASE_ARENASCALE;
	float sideheight = 2*BASE_ARENASCALE;
	float paddleheight = 1*BASE_ARENASCALE;
	float paddlewidth = 3*BASE_ARENASCALE;
	float bottomthickness = 0.5*BASE_ARENASCALE;

	float sidexsize = width/20.f;
	float sideysize = height/20.f;

	string materialbase = "Material.001";
	string materialpaddle = "BaseWhite";
	string sidematerialtall = "Material.001";
	string sidematerialshort = "Material.001";
	string materialclosedpaddlearea = "Material.001";

	// create brushes //

	Leviathan::ObjectLoader* loader = Engine::GetEngine()->GetObjectLoader();

	// WARNING: Huge mess ahead!

	// base surface brush //
	BottomBrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), materialbase, Float3(width, bottomthickness, height), 0.f));
	if(BottomBrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(BottomBrush.get());
		tmp->SetPos(0.f, -bottomthickness, 0.f);
	}
	// arena sides //

	// left top //
	auto tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall, Float3(sidexsize, mheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(-width/2.f+sidexsize/2.f, mheight/2.f, -height/2.f+sideysize/2.f);
	}

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(-width/2.f+sidexsize*1.5f, sideheight/2.f, -height/2.f+sideysize/2.f);
	}

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(-width/2.f+sidexsize/2.f, sideheight/2.f, -height/2.f+sideysize*1.5f);
	}

	// top right //
	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall, Float3(sidexsize, mheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(width/2.f-sidexsize/2.f, mheight/2.f, -height/2.f+sideysize/2.f);
	}

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(width/2.f-sidexsize*1.5f, sideheight/2.f, -height/2.f+sideysize/2.f);
	}

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(width/2.f-sidexsize/2.f, sideheight/2.f, -height/2.f+sideysize*1.5f);
	}


	// bottom left //
	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall, Float3(sidexsize, mheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(-width/2.f+sidexsize/2.f, mheight/2.f, height/2.f-sideysize/2.f);
	}

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(-width/2.f+sidexsize*1.5f, sideheight/2.f, height/2.f-sideysize/2.f);
	}

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(-width/2.f+sidexsize/2.f, sideheight/2.f, height/2.f-sideysize*1.5f);
	}

	// bottom right //
	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall, Float3(sidexsize, mheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(width/2.f-sidexsize/2.f, mheight/2.f, height/2.f-sideysize/2.f);
	}

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(width/2.f-sidexsize*1.5f, sideheight/2.f, height/2.f-sideysize/2.f);
	}

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 0.f));
	if(tmpbrush){
		Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
		tmp->SetPos(width/2.f-sidexsize/2.f, sideheight/2.f, height/2.f-sideysize*1.5f);
	}


	// fill empty paddle spaces //
	if(plycount < 2){

		// fill left with wall //
		tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight/2, sideysize*16.f), 0.f));
		if(tmpbrush){
			Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
			tmp->SetPos(-width/2.f+sidexsize/2.f, sideheight/4.f, 0);
		}

	}
	if(plycount < 3){

		// fill bottom with wall //
		tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize*16.f, sideheight/2, sideysize), 0.f));
		if(tmpbrush){
			Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
			tmp->SetPos(0, sideheight/4.f, height/2.f-sideysize/2.f);
		}
	}
	if(plycount < 4){

		// fill top with wall //
		tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize*16.f, sideheight/2, sideysize), 0.f));
		if(tmpbrush){
			Leviathan::Entity::Brush* tmp = static_cast<Leviathan::Entity::Brush*>(tmpbrush.get());
			tmp->SetPos(0, sideheight/4.f, -height/2.f+sideysize/2.f);
		}
	}


	// paddles //


	// setup joints //



	// link slots to objects //

	// TODO: add prop pointer to slot and allow force input to it //

	// TODO: store own slots for destroying arena for another game //
	return true;
}
// ------------------------------------ //
void Pong::Arena::_ClearPointers(){
	BottomBrush.reset();
}
// ------------------------------------ //

// ------------------------------------ //


