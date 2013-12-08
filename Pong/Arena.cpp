#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_ARENA
#include "Arena.h"
#endif
#include "PongGame.h"
#include "Entities/Objects/Brush.h"
#include "Entities/Objects/Prop.h"
#include "Entities/Objects/TrackEntityController.h"
#include "TextureGenerator.h"
using namespace Pong;
// ------------------------------------ //
Pong::Arena::Arena(shared_ptr<Leviathan::GameWorld> world) : TargetWorld(world), DirectTrail(NULL){

}

Pong::Arena::~Arena(){

}
// ------------------------------------ //
bool Pong::Arena::GenerateArena(PongGame* game, std::vector<PlayerSlot*> &players, int plycount, int maximumsplit, bool clearfirst /*= true*/){
	// check sanity of values //
	QUICKTIME_THISSCOPE;

	if(plycount == 0 || plycount > 4){
		game->SetError("Player count must be over 1");
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

	// Fast access to objects //
	NewtonWorld* nworld = TargetWorld->GetPhysicalWorld()->GetNewtonWorld();
	Leviathan::ObjectLoader* loader = Engine::GetEngine()->GetObjectLoader();

	// These settings are overwritten almost instantly //
	Leviathan::Entity::TrailProperties balltrailproperties(5, 10, 100, false);
	// Set up all elements //
	balltrailproperties.ElementProperties[0] = new Leviathan::Entity::TrailElementProperties(Float4(0), Float4(0.5f, 0.5f, 0.5f, 0), 3.f, 0.3f);

	// Create the trail //
	TrailKeeper = TargetWorld->GetWorldObject(loader->LoadTrailToWorld(TargetWorld.get(), "PongBallTrail", balltrailproperties,	true, &DirectTrail));

	// Set the options with the unified function //
	ColourTheBallTrail(Float4(1.f));


	// calculate sizes //
	float width = 20*BASE_ARENASCALE;
	float height = width;

	float mheight = 3*BASE_ARENASCALE;
	float sideheight = 2*BASE_ARENASCALE;
	float paddleheight = 1*BASE_ARENASCALE;
	float paddlewidth = 3*BASE_ARENASCALE;
	float bottomthickness = 0.5*BASE_ARENASCALE;

	float paddlethicknesswhole = 1*BASE_ARENASCALE; 
	float paddlethicknesssplit = 0.5f*BASE_ARENASCALE;

	float sidexsize = width/20.f;
	float sideysize = height/20.f;

	float paddlemass = 60.f;
	//float paddlemass = 0.f;

	string materialbase = "Material.001";
	string sidematerialtall = "Material.001";
	string sidematerialshort = "Material.001";
	string materialclosedpaddlearea = "Material.001";

	bool infiniteloop = false;

newtonmaterialfetchstartlabel:




	int ArenaMatID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld(L"ArenaMaterial", nworld);
	int PaddleID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld(L"PaddleMaterial", nworld);
	int GoalAreaMatID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld(L"GoalAreaMaterial", nworld);
	int ArenaBaseID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld(L"ArenaBottomMaterial", nworld);

	if(ArenaMatID == -1){
		// All are probably invalid, force world adding //
		assert(!infiniteloop && "Stuck infinitely regenerating materials");
		Logger::Get()->Warning(L"Arena: GenerateArena: world doesn't have materials, regenerating");
		Leviathan::PhysicsMaterialManager::Get()->CreateActualMaterialsForWorld(nworld);
		infiniteloop = true;
		goto newtonmaterialfetchstartlabel;
	}

	// create brushes //



	// WARNING: Huge mess ahead!
	// GetWorldObject is used because the ptr returned by load is not "safe" to use, so we get a shared ptr to the same object, this avoids dynamic
	// cast and is safe at the same time

	// base surface brush //
	Leviathan::Entity::Brush* castedbottombrush;
	BottomBrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), materialbase, Float3(width, bottomthickness, height), 0.f,
		&castedbottombrush));
	castedbottombrush->SetPos(0.f, -bottomthickness/2.f, 0.f);
	castedbottombrush->SetPhysicalMaterialID(ArenaBaseID);

	// Arena ceiling that keeps the ball in //
	auto topbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), "", Float3(width, bottomthickness, height), 0.f,
		&castedbottombrush));
	castedbottombrush->SetPos(0.f, paddleheight+bottomthickness/2.f+BASE_ARENASCALE/2.f, 0.f);
	castedbottombrush->SetPhysicalMaterialID(ArenaBaseID);
	castedbottombrush->SetHiddenState(true);

	// arena sides //

	// left top //
	Leviathan::Entity::Brush* tmp;
	auto tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall, Float3(sidexsize, mheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(-width/2.f+sidexsize/2.f, mheight/2.f, -height/2.f+sideysize/2.f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(-width/2.f+sidexsize*1.5f, sideheight/2.f, -height/2.f+sideysize/2.f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize),
		0.f, &tmp));
	tmp->SetPos(-width/2.f+sidexsize/2.f, sideheight/2.f, -height/2.f+sideysize*1.5f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	// top right //
	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall, Float3(sidexsize, mheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(width/2.f-sidexsize/2.f, mheight/2.f, -height/2.f+sideysize/2.f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(width/2.f-sidexsize*1.5f, sideheight/2.f, -height/2.f+sideysize/2.f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(width/2.f-sidexsize/2.f, sideheight/2.f, -height/2.f+sideysize*1.5f);
	tmp->SetPhysicalMaterialID(ArenaMatID);


	// bottom left //
	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall, Float3(sidexsize, mheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(-width/2.f+sidexsize/2.f, mheight/2.f, height/2.f-sideysize/2.f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(-width/2.f+sidexsize*1.5f, sideheight/2.f, height/2.f-sideysize/2.f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(-width/2.f+sidexsize/2.f, sideheight/2.f, height/2.f-sideysize*1.5f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	// bottom right //
	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall, Float3(sidexsize, mheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(width/2.f-sidexsize/2.f, mheight/2.f, height/2.f-sideysize/2.f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(width/2.f-sidexsize*1.5f, sideheight/2.f, height/2.f-sideysize/2.f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, Float3(sidexsize, sideheight, sideysize), 
		0.f, &tmp));
	tmp->SetPos(width/2.f-sidexsize/2.f, sideheight/2.f, height/2.f-sideysize*1.5f);
	tmp->SetPhysicalMaterialID(ArenaMatID);

	// fill empty paddle spaces //
	for(size_t i = 0; i < players.size(); i++){

		if(!players[i]->IsSlotActive()){
			// The sub slot can save this //
			if(auto split = players[i]->GetSplit())
				if(split->IsSlotActive())
					continue;
			// Fill the empty slot //
			switch(i){
			case 0:
				{
					// fill right with wall //
					tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, 
						Float3(sidexsize, sideheight/2, sideysize*16.f), 0.f, &tmp));
					tmp->SetPos(width/2.f-sidexsize/2.f, sideheight/4.f, 0);
					tmp->SetPhysicalMaterialID(ArenaMatID);
				}
				break;
			case 1:
				{
					// fill bottom with wall //
					tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, 
						Float3(sidexsize*16.f, sideheight/2, sideysize), 0.f, &tmp));
					tmp->SetPos(0, sideheight/4.f, height/2.f-sideysize/2.f);
					tmp->SetPhysicalMaterialID(ArenaMatID);
				}
				break;
			case 2:
				{
					// fill left with wall //
					tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, 
						Float3(sidexsize, sideheight/2, sideysize*16.f), 0.f, &tmp));
					tmp->SetPos(-width/2.f+sidexsize/2.f, sideheight/4.f, 0);
					tmp->SetPhysicalMaterialID(ArenaMatID);
				}
				break;
			case 3:
				{
					// fill top with wall //
					tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort, 
						Float3(sidexsize*16.f, sideheight/2, sideysize), 0.f, &tmp));
					tmp->SetPos(0, sideheight/4.f, -height/2.f+sideysize/2.f);
					tmp->SetPhysicalMaterialID(ArenaMatID);
				}
				break;

			}

		}
	}

	// paddles and link slots to objects//

	// loop through players and add paddles //
	for(size_t i = 0; i < players.size(); i++){
		// skip empty slots //
		if(!players[i]->IsSlotActive())
			continue;
		bool secondary = false;
addplayerpaddlelabel:

		bool splitslotopen = false;
		if(players[i]->GetSplit())
			splitslotopen = players[i]->GetSplit()->IsSlotActive();

		// Choose the thickness based on the split count of THIS slot //
		float paddlethickness = secondary || splitslotopen ? paddlethicknesssplit: paddlethicknesswhole;

		// Get the colour for the paddle //
		Float4 colour = secondary ? players[i]->GetSplit()->GetColour(): players[i]->GetColour();

		// add paddle based on loop index //
		auto plypaddle = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), GetMaterialNameForPlayerColour(colour), 
			Float3((i == 0 || i == 2) ? paddlethickness: paddlewidth, paddleheight, (i == 0 || i == 2) ? paddlewidth: paddlethickness), paddlemass,
			&tmp));
		// setup position //
		float horiadjust = 0;
		if(secondary || splitslotopen)
			horiadjust = secondary ? 0: paddlethickness;

		switch(i){
		case 0: tmp->SetPos(width/2.f-paddlethickness/2.f-horiadjust, paddleheight/2.f, 0); break;
		case 1: tmp->SetPos(0, paddleheight/2.f, width/2.f-paddlethickness/2.f-horiadjust); break;
		case 2: tmp->SetPos(-width/2.f+paddlethickness/2.f+horiadjust, paddleheight/2.f, 0);break;
		case 3: tmp->SetPos(0, paddleheight/2.f, -width/2.f+paddlethickness/2.f+horiadjust); break;
		}
		tmp->SetPhysicalMaterialID(PaddleID);

		// setup joints //
		if(!tmp->CreateConstraintWith<Leviathan::Entity::SliderConstraint>(castedbottombrush)->SetParameters(
			(i == 0 || i == 2) ? Float3(0.f, 0.f, -1.f): Float3(1.f, 0.f, 0.f))->Init()){
				Logger::Get()->Error(L"Arena: GenerateArena: failed to create slider for paddle "+Convert::ToWstring(i+1));
		}

		// link //
		secondary ? players[i]->GetSplit()->SetPaddleObject(plypaddle): players[i]->SetPaddleObject(plypaddle);

		// Create the track controller //
		std::vector<Leviathan::Entity::TrackControllerPosition> MovementPositions(2);

		switch(i){
		case 0:
			{
				MovementPositions[0] = Leviathan::Entity::TrackControllerPosition(
					Float3(width/2.f-paddlethickness/2.f-horiadjust, paddleheight/2.f, height/2.f-sideysize*2-paddlewidth/2.f), Float4::IdentityQuaternion());
				MovementPositions[1] = Leviathan::Entity::TrackControllerPosition(
					Float3(width/2.f-paddlethickness/2.f-horiadjust, paddleheight/2.f, -height/2.f+sideysize*2+paddlewidth/2.f), Float4::IdentityQuaternion());
			}
			break;
		case 1:
			{
				MovementPositions[0] = Leviathan::Entity::TrackControllerPosition(
					Float3(width/2.f-sidexsize*2-paddlewidth/2.f, paddleheight/2.f, width/2.f-paddlethickness/2.f-horiadjust), Float4::IdentityQuaternion());
				MovementPositions[1] = Leviathan::Entity::TrackControllerPosition(
					Float3(-width/2.f+sidexsize*2+paddlewidth/2.f, paddleheight/2.f, width/2.f-paddlethickness/2.f-horiadjust), Float4::IdentityQuaternion());
			}
			break;
		case 2:
			{
				MovementPositions[0] = Leviathan::Entity::TrackControllerPosition(
					Float3(-width/2.f+paddlethickness/2.f+horiadjust, paddleheight/2.f, height/2.f-sideysize*2-paddlewidth/2.f), Float4::IdentityQuaternion());
				MovementPositions[1] = Leviathan::Entity::TrackControllerPosition(
					Float3(-width/2.f+paddlethickness/2.f+horiadjust, paddleheight/2.f, -height/2.f+sideysize*2+paddlewidth/2.f), Float4::IdentityQuaternion());
			}
			break;
		case 3:
			{
				MovementPositions[0] = Leviathan::Entity::TrackControllerPosition(
					Float3(width/2.f-sidexsize*2-paddlewidth/2.f, paddleheight/2.f, -width/2.f+paddlethickness/2.f+horiadjust), Float4::IdentityQuaternion());
				MovementPositions[1] = Leviathan::Entity::TrackControllerPosition(
					Float3(-width/2.f+sidexsize*2+paddlewidth/2.f, paddleheight/2.f, -width/2.f+paddlethickness/2.f+horiadjust), Float4::IdentityQuaternion());
			}
			break;
		}
		Leviathan::Entity::TrackEntityController* controller;
		auto track = TargetWorld->GetWorldObject(loader->LoadTrackEntityControllerToWorld(TargetWorld.get(), MovementPositions, tmp, &controller));

		// Set //
		secondary ? players[i]->GetSplit()->SetTrackObject(track, controller): players[i]->SetTrackObject(track, controller);

		// Paddle should be in the middle by default, so set progress to 50% //
		controller->SetProgressTowardsNextNode(0.5f);

		if(secondary)
			continue;
		// Create goal area for this slot //
		auto goalarea = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), materialclosedpaddlearea, 
			Float3((i == 0 || i == 2) ? paddlethickness: width, sideheight, (i == 0 || i == 2) ? height: paddlethickness), 0.f, &tmp));
		tmp->SetPhysicalMaterialID(GoalAreaMatID);
		tmp->SetHiddenState(true);
		
		switch(i){
		case 0: tmp->SetPos(width/2.f+paddlethickness/2.f, sideheight/2.f, 0); break;
		case 1: tmp->SetPos(0, sideheight/2.f, width/2.f+paddlethickness/2.f); break;
		case 2: tmp->SetPos(-width/2.f-paddlethickness/2.f, sideheight/2.f, 0);break;
		case 3: tmp->SetPos(0, sideheight/2.f, -width/2.f-paddlethickness/2.f); break;
		}

		// Set to slot //
		players[i]->SetGoalAreaObject(goalarea);

		// loop again if has secondary //
		if(players[i]->GetSplit() != NULL){
			secondary = true;
			goto addplayerpaddlelabel;
		}
	}


	return true;
}
// ------------------------------------ //
void Pong::Arena::_ClearPointers(){
	BottomBrush.reset();
	TrailKeeper.reset();
	DirectTrail = NULL;
}

void Pong::Arena::ServeBall(){
	if(Ball){
		// destroy old //
		TargetWorld->DestroyObject(Ball->GetID());
		Ball.reset();
	}

	// we want to load our ball prop into the world //
	Leviathan::Entity::Prop* prop;
	Ball = TargetWorld->GetWorldObject(Leviathan::Engine::Get()->GetObjectLoader()->LoadPropToWorld(TargetWorld.get(), L"PongBall", &prop));

	// set to center of board //
	prop->SetPos(Float3(0.f, 0.5f, 0.f));

	// Set material //
	prop->SetPhysicalMaterial(L"BallMaterial");

	// Parent the trail to the ball //
	DirectTrail->AddNonPhysicsParent(prop);

	// Update trail colour //
	ColourTheBallTrail(Float4(1));

	Float3 dir(0);

	// Add some randomness //
	int count = Leviathan::Random::Get()->GetNumber(0, 15);

	PongGame* game = PongGame::Get();

	while(count > -1){

		for(size_t i = 0; i < game->PlayerList.size(); i++){

			if(game->PlayerList[i]->IsSlotActive()){
				count--;
				if(count < 0){
					// Set direction //

					switch(i){
					case 0: dir = Float3(1.f, 0.f, 0.f); break;
					case 1: dir = Float3(0.f, 0.f, 1.f); break;
					case 2: dir = Float3(-1.f, 0.f, 0.f); break;
					case 3: dir = Float3(0.f, 0.f, -1.f); break;
					}

					break;
				}
			}
		}
	}
	
	// Add base speed //
	dir *= 25.f;


	// TODO: queue send event //
	prop->GiveImpulse(dir);
}
// ------------------------------------ //
void Pong::Arena::GiveBallSpeed(float mult){
	if(Ball){

		// Cast //
		Leviathan::Entity::Prop* tmpball = dynamic_cast<Leviathan::Entity::Prop*>(Ball.get());
		if(tmpball != NULL){
			// Get current velocity //
			Float3 targetspeed = tmpball->GetBodyVelocity();
			// Don't want to apply any Y directional force //
			targetspeed.Y = 0;
			//// We want the direction //
			//targetspeed = targetspeed.Normalize();
			// Add the factor //
			targetspeed *= mult;

			// Limit maximum speed //
			if(targetspeed.HAddAbs() >= BALL_SPEED_MAX){

				tmpball->RemoveApplyForce(L"BallPush");
			} else {
				tmpball->ApplyForce(new ApplyForceInfo(targetspeed, true, true, new wstring(L"BallPush")));
			}
		}
	}
}

void Pong::Arena::LetGoOfBall(){
	// We should delete it (but after this physics update is done) //
	if(Ball){
		Ball->GetWorld()->QueueDestroyObject(Ball->GetID());
		Ball.reset();
	}
}

bool Pong::Arena::IsBallInPaddleArea(){

	// Cast //
	Leviathan::Entity::Prop* tmpball = dynamic_cast<Leviathan::Entity::Prop*>(Ball.get());

	if(tmpball == NULL)
		return false;
	
	// Get pos //
	Float3 pos = tmpball->GetPos();

	if(abs(pos.X) >= 8.5f*BASE_ARENASCALE || abs(pos.Z) >= 8.5f*BASE_ARENASCALE)
		return true;
	return false;
}
// ------------------------------------ //
void Pong::Arena::ColourTheBallTrail(const Float4 &colour){
	// Adjust the trail parameters //
	Leviathan::Entity::TrailProperties balltrailproperties(5, 10, 100, false);
	// Set up all elements //
	balltrailproperties.ElementProperties[0] = new Leviathan::Entity::TrailElementProperties(colour, Float4(0.7f, 0.7f, 0.7f, 0.3f), 5.f, 0.6f);

	if(DirectTrail){

		DirectTrail->SetTrailProperties(balltrailproperties);
	}
}
// ------------------------------------ //
std::string Pong::Arena::GetMaterialNameForPlayerColour(const Float4 &colour){

	auto iter = ColourMaterialName.find(colour);

	if(iter != ColourMaterialName.end()){

		return iter->second;
	}

	// Create new //
	string generatename = "Colour_"+Convert::Float4ToString<stringstream, string>(colour)+"_PaddleMaterial";

	// Generate //
	Leviathan::TextureGenerator::LoadSolidColourLightMaterialToMemory(generatename, colour);

	ColourMaterialName[colour] = generatename;
	return generatename;
}

