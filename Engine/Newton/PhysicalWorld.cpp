#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_PHYSICALWORLD
#include "PhysicalWorld.h"
#endif
#include <Newton.h>
#include "PhysicalMaterialManager.h"
#include "Events/EventHandler.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::PhysicalWorld::PhysicalWorld(GameWorld* owner) : LastSimulatedTime(0), PassedTimeTotal(0), OwningWorld(owner){

	// create newton world //
	World = NewtonCreate();

	// set physics accuracy //
	// most accurate mode //

	// TODO: figure out how to use this exact mode //
	//NewtonSetSolverModel(World, 0);
	//// fast mode //
	//NewtonSetSolverModel(World, 1);
	NewtonSetSolverModel(World, 2);

	// set size //
	//NewtonSetWorldSize();

	// Create materials for this world //
	PhysicsMaterialManager::Get()->CreateActualMaterialsForWorld(World);
}

DLLEXPORT Leviathan::PhysicalWorld::~PhysicalWorld(){
	// finally destroy the newton world
	//NewtonDestroyAllBodies(World);
	NewtonDestroy(World);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::SimulateWorld(){
	// calculate passed delta time and simulate if possible //

	__int64 curtime = Misc::GetTimeMicro64();
	// calculate passed time and reset //
	PassedTimeTotal += (int)(curtime-LastSimulatedTime);
	LastSimulatedTime = curtime;

	// simulate updates //
	while(PassedTimeTotal >= NEWTON_FPS_IN_MICROSECONDS){
		// avoid freezing the program //
		if(PassedTimeTotal >= 100000){
			Logger::Get()->Warning(L"PhysicalWorld: SimulateWorld: falling behind, entering simulated time ("+Convert::ToWstring(PassedTimeTotal)+
				L" is over 100000 microseconds)");
			PassedTimeTotal = 100000;
		}

		// Call event //
		EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_PHYSICS_BEGIN, new PhysicsStartEventData(NEWTON_TIMESTEP, OwningWorld)));

		NewtonUpdate(World, NEWTON_TIMESTEP);
		PassedTimeTotal -= (int)NEWTON_FPS_IN_MICROSECONDS;
	}
}

DLLEXPORT void Leviathan::PhysicalWorld::ClearTimers(){
	LastSimulatedTime = Misc::GetTimeMicro64();
	PassedTimeTotal = 0;
}
// ------------------------------------ //
DLLEXPORT NewtonWorld* Leviathan::PhysicalWorld::GetNewtonWorld(){
	return World;
}
// ------------------------------------ //

// ------------------------------------ //


