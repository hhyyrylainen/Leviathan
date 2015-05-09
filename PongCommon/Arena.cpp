#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_ARENA
#include "Arena.h"
#endif
#include "CommonPong.h"
#include "Entities/Objects/Brush.h"
#include "Entities/Objects/Prop.h"
#include "Entities/Objects/TrackEntityController.h"
#include "TextureGenerator.h"
#include "Handlers/ObjectLoader.h"
#include "Newton/PhysicsMaterialManager.h"
#include "Utility/Random.h"
#include "PongConstraints.h"
using namespace Pong;
// ------------------------------------ //
Pong::Arena::Arena(shared_ptr<Leviathan::GameWorld> world) : TargetWorld(world), DirectTrail(NULL){

}

Pong::Arena::~Arena(){

}
// ------------------------------------ //
bool Pong::Arena::GenerateArena(BasePongParts* game, PlayerList &plys){

	QUICKTIME_THISSCOPE;
    GUARD_LOCK();
	
	std::vector<PlayerSlot*>& plyvec = plys.GetVec();
	
	// Check sanity of values //	
	if(plyvec.empty() || plyvec.size() > 4){
		game->SetError("Player count must be over 1");
		return false;
	}
	
	_ClearPointers(guard);
	

	// Fast access to objects //
	NewtonWorld* nworld = TargetWorld->GetPhysicalWorld()->GetNewtonWorld();
	Leviathan::ObjectLoader* loader = Engine::GetEngine()->GetObjectLoader();

    VerifyTrail(guard);
    
	// Set the options with the unified function //
	ColourTheBallTrail(guard, Float4(1.f));


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




	int ArenaMatID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld("ArenaMaterial", nworld);
	int PaddleID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld("PaddleMaterial", nworld);
	int GoalAreaMatID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld("GoalAreaMaterial", nworld);
	int ArenaBaseID = Leviathan::PhysicsMaterialManager::Get()->GetMaterialIDForWorld("ArenaBottomMaterial", nworld);

	if(ArenaMatID == -1){
		// All are probably invalid, force world adding //
		assert(!infiniteloop && "Stuck infinitely regenerating materials");
		Logger::Get()->Warning("Arena: GenerateArena: world doesn't have materials, regenerating");
		Leviathan::PhysicsMaterialManager::Get()->CreateActualMaterialsForWorld(nworld);
		infiniteloop = true;
		goto newtonmaterialfetchstartlabel;
	}

	// create brushes //



	// WARNING: Huge mess ahead!
	// GetWorldObject is used because the ptr returned by load is not "safe" to use, so we get a shared ptr
    // to the same object, this avoids dynamic cast and is safe at the same time

	// base surface brush //
	Leviathan::Entity::Brush* castedbottombrush;
	BottomBrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), materialbase,
            Float3(width, bottomthickness, height), 0.f, ArenaBaseID, &castedbottombrush));
    
	castedbottombrush->SetPosComponents(0.f, -bottomthickness/2.f, 0.f);
    

	// Arena ceiling that keeps the ball in //
	auto topbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), "",
            Float3(width, bottomthickness, height), 0.f, ArenaBaseID, &castedbottombrush));
    
	castedbottombrush->SetPosComponents(0.f, paddleheight+bottomthickness/2.f+BASE_ARENASCALE/2.f, 0.f);
	castedbottombrush->SetHiddenState(true);

	// arena sides //

	// left top //
	Leviathan::Entity::Brush* tmp;
	auto tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall,
            Float3(sidexsize, mheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(-width/2.f+sidexsize/2.f, mheight/2.f, -height/2.f+sideysize/2.f);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort,
            Float3(sidexsize, sideheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(-width/2.f+sidexsize*1.5f, sideheight/2.f, -height/2.f+sideysize/2.f);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort,
            Float3(sidexsize, sideheight, sideysize),
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(-width/2.f+sidexsize/2.f, sideheight/2.f, -height/2.f+sideysize*1.5f);

	// top right //
	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall,
            Float3(sidexsize, mheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(width/2.f-sidexsize/2.f, mheight/2.f, -height/2.f+sideysize/2.f);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort,
            Float3(sidexsize, sideheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(width/2.f-sidexsize*1.5f, sideheight/2.f, -height/2.f+sideysize/2.f);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort,
            Float3(sidexsize, sideheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(width/2.f-sidexsize/2.f, sideheight/2.f, -height/2.f+sideysize*1.5f);


	// bottom left //
	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall,
            Float3(sidexsize, mheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(-width/2.f+sidexsize/2.f, mheight/2.f, height/2.f-sideysize/2.f);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort,
            Float3(sidexsize, sideheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(-width/2.f+sidexsize*1.5f, sideheight/2.f, height/2.f-sideysize/2.f);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort,
            Float3(sidexsize, sideheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(-width/2.f+sidexsize/2.f, sideheight/2.f, height/2.f-sideysize*1.5f);

	// bottom right //
	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialtall,
            Float3(sidexsize, mheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(width/2.f-sidexsize/2.f, mheight/2.f, height/2.f-sideysize/2.f);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort,
            Float3(sidexsize, sideheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(width/2.f-sidexsize*1.5f, sideheight/2.f, height/2.f-sideysize/2.f);

	tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(), sidematerialshort,
            Float3(sidexsize, sideheight, sideysize), 
            0.f, ArenaMatID, &tmp));
	tmp->SetPosComponents(width/2.f-sidexsize/2.f, sideheight/2.f, height/2.f-sideysize*1.5f);
	
	
	// fill empty paddle spaces //
	for(size_t i = 0; i < plyvec.size(); i++){

		if(!plyvec[i]->IsSlotActive()){
			// The sub slot can save this //
			if(auto split = plyvec[i]->GetSplit())
				if(split->IsSlotActive())
					continue;
			// Fill the empty slot //
			switch(i){
			case 0:
				{
					// fill right with wall //
					tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(),
                            sidematerialshort, Float3(sidexsize, sideheight/2, sideysize*16.f), 0.f, ArenaMatID, &tmp));
					tmp->SetPosComponents(width/2.f-sidexsize/2.f, sideheight/4.f, 0);
				}
				break;
			case 1:
				{
					// fill bottom with wall //
					tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(),
                            sidematerialshort, Float3(sidexsize*16.f, sideheight/2, sideysize), 0.f, ArenaMatID, &tmp));
					tmp->SetPosComponents(0, sideheight/4.f, height/2.f-sideysize/2.f);
				}
				break;
			case 2:
				{
					// fill left with wall //
					tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(),
                            sidematerialshort, Float3(sidexsize, sideheight/2, sideysize*16.f), 0.f, ArenaMatID, &tmp));
					tmp->SetPosComponents(-width/2.f+sidexsize/2.f, sideheight/4.f, 0);
				}
				break;
			case 3:
				{
					// fill top with wall //
					tmpbrush = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(),
                            sidematerialshort, Float3(sidexsize*16.f, sideheight/2, sideysize), 0.f, ArenaMatID, &tmp));
					tmp->SetPosComponents(0, sideheight/4.f, -height/2.f+sideysize/2.f);
				}
				break;

			}

		}
	}

	// paddles and link slots to objects//

	// loop through players and add paddles //
	for(size_t i = 0; i < plyvec.size(); i++){
		// skip empty slots //
		if(!plyvec[i]->IsSlotActive())
			continue;
		bool secondary = false;
addplayerpaddlelabel:

		bool splitslotopen = false;
		if(plyvec[i]->GetSplit())
			splitslotopen = plyvec[i]->GetSplit()->IsSlotActive();

		// Choose the thickness based on the split count of THIS slot //
		float paddlethickness = secondary || splitslotopen ? paddlethicknesssplit: paddlethicknesswhole;

		// Get the colour for the paddle //
		Float4 colour = secondary ? plyvec[i]->GetSplit()->GetColour(): plyvec[i]->GetColour();

		// add paddle based on loop index //
		auto plypaddle = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(),
                GetMaterialNameForPlayerColour(colour), Float3((i == 0 || i == 2) ? paddlethickness: paddlewidth,
                    paddleheight, (i == 0 || i == 2) ? paddlewidth: paddlethickness), paddlemass, PaddleID, &tmp));
        
		// setup position //
		float horiadjust = 0;
		if(secondary || splitslotopen)
			horiadjust = secondary ? 0: paddlethickness;

		switch(i){
            case 0: tmp->SetPosComponents(width/2.f-paddlethickness/2.f-horiadjust, paddleheight/2.f, 0); break;
            case 1: tmp->SetPosComponents(0, paddleheight/2.f, width/2.f-paddlethickness/2.f-horiadjust); break;
            case 2: tmp->SetPosComponents(-width/2.f+paddlethickness/2.f+horiadjust, paddleheight/2.f, 0); break;
            case 3: tmp->SetPosComponents(0, paddleheight/2.f, -width/2.f+paddlethickness/2.f+horiadjust); break;
		}

		// setup joints //
		if(!tmp->CreateConstraintWith<Leviathan::Entity::SliderConstraint>(castedbottombrush)->SetParameters(
                (i == 0 || i == 2) ? Float3(0.f, 0.f, -1.f): Float3(1.f, 0.f, 0.f))->Init())
        {
            Logger::Get()->Error("Arena: GenerateArena: failed to create slider for paddle "+
                Convert::ToString(i+1));
		}

		// link //
		int plynumber = secondary ? plyvec[i]->GetSplit()->GetPlayerNumber(): plyvec[i]->GetPlayerNumber();

        // tmp should still be plypaddle
        tmp->CreateConstraintWith<EmotionalConnection>(NULL)->SetParameters(plynumber, LINK_TYPE_PADDLE)->Init();

		// Create the track controller //
		std::vector<Leviathan::Entity::TrackControllerPosition> MovementPositions(2);

		switch(i){
		case 0:
			{
				MovementPositions[0] = Leviathan::Entity::TrackControllerPosition(
					Float3(width/2.f-paddlethickness/2.f-horiadjust, paddleheight/2.f,
                        height/2.f-sideysize*2-paddlewidth/2.f), Float4::IdentityQuaternion());
				MovementPositions[1] = Leviathan::Entity::TrackControllerPosition(
					Float3(width/2.f-paddlethickness/2.f-horiadjust, paddleheight/2.f,
                        -height/2.f+sideysize*2+paddlewidth/2.f), Float4::IdentityQuaternion());
			}
			break;
		case 1:
			{
				MovementPositions[0] = Leviathan::Entity::TrackControllerPosition(
					Float3(width/2.f-sidexsize*2-paddlewidth/2.f, paddleheight/2.f,
                        width/2.f-paddlethickness/2.f-horiadjust), Float4::IdentityQuaternion());
				MovementPositions[1] = Leviathan::Entity::TrackControllerPosition(
					Float3(-width/2.f+sidexsize*2+paddlewidth/2.f, paddleheight/2.f,
                        width/2.f-paddlethickness/2.f-horiadjust), Float4::IdentityQuaternion());
			}
			break;
		case 2:
			{
				MovementPositions[0] = Leviathan::Entity::TrackControllerPosition(
					Float3(-width/2.f+paddlethickness/2.f+horiadjust, paddleheight/2.f,
                        height/2.f-sideysize*2-paddlewidth/2.f), Float4::IdentityQuaternion());
				MovementPositions[1] = Leviathan::Entity::TrackControllerPosition(
					Float3(-width/2.f+paddlethickness/2.f+horiadjust, paddleheight/2.f,
                        -height/2.f+sideysize*2+paddlewidth/2.f), Float4::IdentityQuaternion());
			}
			break;
		case 3:
			{
				MovementPositions[0] = Leviathan::Entity::TrackControllerPosition(
					Float3(width/2.f-sidexsize*2-paddlewidth/2.f, paddleheight/2.f,
                        -width/2.f+paddlethickness/2.f+horiadjust), Float4::IdentityQuaternion());
				MovementPositions[1] = Leviathan::Entity::TrackControllerPosition(
					Float3(-width/2.f+sidexsize*2+paddlewidth/2.f, paddleheight/2.f,
                        -width/2.f+paddlethickness/2.f+horiadjust), Float4::IdentityQuaternion());
			}
			break;
		}
		Leviathan::Entity::TrackEntityController* controller;
		auto track = TargetWorld->GetWorldObject(loader->LoadTrackEntityControllerToWorld(TargetWorld.get(),
                MovementPositions, tmp, &controller));

		// Set //
        plynumber = secondary ? plyvec[i]->GetSplit()->GetPlayerNumber(): plyvec[i]->GetPlayerNumber();
        
        controller->CreateConstraintWith<EmotionalConnection>(NULL)->SetParameters(plynumber, LINK_TYPE_TRACK)->Init();

		// Paddle should be in the middle by default, so set progress to 50% //
		controller->SetProgressTowardsNextNode(0.5f);

        // Connect the paddle to the track //
        controller->CreateConstraintWith<Leviathan::Entity::ControllerConstraint>(
            dynamic_cast<Leviathan::BaseConstraintable*>(
                plypaddle.get()))->Init();

		if(secondary)
			continue;
        
		// Create goal area for this slot //
		auto goalarea = TargetWorld->GetWorldObject(loader->LoadBrushToWorld(TargetWorld.get(),
                materialclosedpaddlearea, 
                Float3((i == 0 || i == 2) ? paddlethickness: width, sideheight,
                    (i == 0 || i == 2) ? height: paddlethickness), 0.f, GoalAreaMatID, &tmp));
		tmp->SetHiddenState(true);
		
		switch(i){
            case 0: tmp->SetPosComponents(width/2.f+paddlethickness/2.f, sideheight/2.f, 0); break;
            case 1: tmp->SetPosComponents(0, sideheight/2.f, width/2.f+paddlethickness/2.f); break;
            case 2: tmp->SetPosComponents(-width/2.f-paddlethickness/2.f, sideheight/2.f, 0); break;
            case 3: tmp->SetPosComponents(0, sideheight/2.f, -width/2.f-paddlethickness/2.f); break;
		}

		// Set to slot //
        tmp->CreateConstraintWith<EmotionalConnection>(NULL)->SetParameters(plynumber, LINK_TYPE_GOAL)->Init();

        Logger::Get()->Write("Created goal area id: "+Convert::ToString(tmp->GetID()));

		// loop again if has secondary //
		if(plyvec[i]->GetSplit() != NULL){
			secondary = true;
			goto addplayerpaddlelabel;
		}
	}

    // Notify how much stuff was created //
    Logger::Get()->Info("Arena: finished generating arena, total objects: "+
        Convert::ToString(TargetWorld->GetObjectCount()));
    
	return true;
}
// ------------------------------------ //
void Pong::Arena::_ClearPointers(Lock &guard){

	BottomBrush.reset();
	TrailKeeper.reset();
	DirectTrail = NULL;
}

void Pong::Arena::ServeBall(){

    LetGoOfBall();

    GUARD_LOCK();


	// we want to load our ball prop into the world //
	Leviathan::Entity::Prop* prop;
    
    
	auto tempball = TargetWorld->GetWorldObject(Leviathan::Engine::Get()->GetObjectLoader()->LoadPropToWorld(
            TargetWorld.get(), "PongBall", TargetWorld->GetPhysicalMaterial("BallMaterial"),
            &prop));

    assert(tempball && prop && "failed to load the Ball model");

    // Make the ball be the actual ball //
    prop->CreateConstraintWith<GameBallConnection>(NULL)->Init(guard);
    
    // Verify that the constraint was created //
    assert(Ball == tempball && "Failed to create emotional connection between the ball and NUL");
        
	// set to center of board //
	prop->SetPos(Float3(0.f, 0.5f, 0.f));

	// Parent the trail to the ball //
    // TODO: make this send it to the client //
	DirectTrail->AddNonPhysicsParent(prop);

	// Update trail colour //
	ColourTheBallTrail(guard, Float4(1));

	Float3 dir(0);

	// Add some randomness //
	int count = Leviathan::Random::Get()->GetNumber(0, 15);

	BasePongParts* game = BasePongParts::Get();

    // TODO: remove this debug code
    bool someactive = 0;
    for(size_t i = 0; i < game->_PlayerList.Size(); i++){

        if(game->_PlayerList[i]->IsSlotActive()){

            someactive = true;
        }
    }
    
    if(!someactive)
        Logger::Get()->Error("NO ACTIVE SLOTS; WILL DEADLOCK");
    //

	while(count > -1){

		for(size_t i = 0; i < game->_PlayerList.Size(); i++){

			if(game->_PlayerList[i]->IsSlotActive()){
				count--;
				if(count < 0){
					// Set direction //

					switch(i){
                        case 0: dir = Float3(25.f, 0.f, 0.f)+(Leviathan::Random::Get()->GetNumber(0, 1) ?
                            Float3(0.f, 0.f, 1.f): Float3(0.f, 0.f, -1.f)); break;
                        case 1: dir = Float3(0.f, 0.f, 25.f)+(Leviathan::Random::Get()->GetNumber(0, 1) ?
                            Float3(1.f, 0.f, 0.f): Float3(-1.f, 0.f, 0.f)); break;
                        case 2: dir = Float3(-25.f, 0.f, 0.f)+(Leviathan::Random::Get()->GetNumber(0, 1) ?
                            Float3(0.f, 0.f, 1.f): Float3(0.f, 0.f, -1.f)); break;
                        case 3: dir = Float3(0.f, 0.f, -25.f)+(Leviathan::Random::Get()->GetNumber(0, 1) ?
                            Float3(1.f, 0.f, 0.f): Float3(-1.f, 0.f, 0.f)); break;
					}

					break;
				}
			}
		}
	}


	prop->GiveImpulse(dir);

    // We changed a few things so notify all receivers //
    prop->SendUpdatesToAllClients(TargetWorld->GetTickNumber());
}
// ------------------------------------ //
void Pong::Arena::VerifyTrail(Lock &guard){
    
    if(DirectTrail)
        return;
    
    // These settings are overwritten almost instantly //
	Leviathan::Entity::TrailProperties balltrailproperties(5, 10, 100, false);
	
	// Set up all elements //
	balltrailproperties.ElementProperties[0] = new Leviathan::Entity::TrailElementProperties(
        Float4(0), Float4(0.5f, 0.5f, 0.5f, 0), 3.f, 0.3f);

    // Create the trail //
	TrailKeeper = TargetWorld->GetWorldObject(Leviathan::Engine::Get()->GetObjectLoader()->LoadTrailToWorld(
            TargetWorld.get(), "PongBallTrail", balltrailproperties, true, &DirectTrail));
}
// ------------------------------------ //
void Pong::Arena::LetGoOfBall(){
    GUARD_LOCK();
    
	// We should delete it (but after this physics update is done) //
	if(Ball && TargetWorld){
        
        Logger::Get()->Info("Arena: destroying old ball");
		TargetWorld->QueueDestroyObject(Ball->GetID());
		Ball.reset();
	}
}

bool Pong::Arena::IsBallInPaddleArea(){

    GUARD_LOCK();
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
void Pong::Arena::ColourTheBallTrail(Lock &guard, const Float4 &colour){
	// Adjust the trail parameters //
	Leviathan::Entity::TrailProperties balltrailproperties(5, 10, 100, false);
	// Set up all elements //
	balltrailproperties.ElementProperties[0] = new Leviathan::Entity::TrailElementProperties(colour,
        Float4(0.7f, 0.7f, 0.7f, 0.3f), 5.f, 0.6f);

	if(DirectTrail){

		DirectTrail->SetTrailProperties(balltrailproperties);
	}
}
// ------------------------------------ //
std::string Pong::Arena::GetMaterialNameForPlayerColour(const Float4 &colour){

    Lock lock(ColourMaterialNameMutex);
    
	auto iter = ColourMaterialName.find(colour);

	if(iter != ColourMaterialName.end()){

		return iter->second;
	}

	// Create new //
    // The prefix in the name allows the clients to generate the texture when they need to use it
	string generatename = "/Generated/Colour_"+Convert::Float4ToString<stringstream, string>(colour)+"_PaddleMaterial";

	// Generate (only in graphical mode) //
    if(!Engine::Get()->GetNoGui())
        Leviathan::TextureGenerator::LoadSolidColourLightMaterialToMemory(generatename, colour);

	ColourMaterialName[colour] = generatename;
	return generatename;
}

