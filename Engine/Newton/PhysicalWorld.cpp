#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_PHYSICALWORLD
#include "PhysicalWorld.h"
#endif
#include <Newton.h>
#include "PhysicsMaterialManager.h"
#include "Events/EventHandler.h"
#include "Common/Misc.h"
#include "boost/thread/lock_types.hpp"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::PhysicalWorld::PhysicalWorld(GameWorld* owner) :
    LastSimulatedTime(0), PassedTimeTotal(0), OwningWorld(owner), ResimulatedBody(NULL)
{

	// create newton world //
	World = NewtonCreate();

	// set physics accuracy //
	// most accurate mode //

	// \todo figure out how to use this exact mode //
	//NewtonSetSolverModel(World, 0);

    // Accurate enough mode //
    NewtonSetSolverModel(World, 1);

    // Set us as the user data //
    NewtonWorldSetUserData(World, this);

	// Create materials for this world //
	PhysicsMaterialManager::Get()->CreateActualMaterialsForWorld(World);
}

DLLEXPORT Leviathan::PhysicalWorld::~PhysicalWorld(){

    NewtonWorldSetUserData(World, NULL);
    
	// finally destroy the newton world
    
	NewtonDestroy(World);
    World = NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::SimulateWorld(){
	// calculate passed delta time and simulate if possible //

	__int64 curtime = Misc::GetTimeMicro64();
	// calculate passed time and reset //
	PassedTimeTotal += (int)(curtime-LastSimulatedTime);
	LastSimulatedTime = curtime;

    boost::unique_lock<boost::mutex> lock(WorldUpdateLock);
    
	// simulate updates //
	while(PassedTimeTotal >= NEWTON_FPS_IN_MICROSECONDS){
		// avoid freezing the program //
		if(PassedTimeTotal >= 10000*NEWTON_FPS_IN_MICROSECONDS){

            Logger::Get()->Warning("Game pretty much will deadlock now in physical update, passed time: "+
                Convert::ToString(PassedTimeTotal));
			PassedTimeTotal = 1000*NEWTON_FPS_IN_MICROSECONDS;
		}

		// Call event //
		EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_PHYSICS_BEGIN, new PhysicsStartEventData(NEWTON_TIMESTEP,
                    OwningWorld)));

		NewtonUpdate(World, NEWTON_TIMESTEP);
		PassedTimeTotal -= (int)NEWTON_FPS_IN_MICROSECONDS;
	}
}
// ------------------------------------ //
int Leviathan::SingleBodyUpdate(const NewtonWorld* const newtonWorld, const void* islandHandle,
    int bodyCount)
{

    PhysicalWorld* pworld = reinterpret_cast<PhysicalWorld*>(NewtonWorldGetUserData(newtonWorld));
    
    for(int i = 0; i < bodyCount; i++){

        if(NewtonIslandGetBody(islandHandle, i) == pworld->ResimulatedBody){
            
            // Target body is part of this collision, simulate it //
            return 1;
        }
    }
    
    // Wasn't the target body, ignore //
    return 0;
}

DLLEXPORT void Leviathan::PhysicalWorld::ResimulateBody(NewtonBody* body, int milliseconds){

    int simulateruns = (0.001f*milliseconds)/NEWTON_TIMESTEP;

    DEBUG_BREAK;
    
    boost::unique_lock<boost::mutex> lock(WorldUpdateLock);

    ResimulatedBody = body;
    
    // Setup single island callbacks //
    NewtonSetIslandUpdateEvent(World, &SingleBodyUpdate);

    for(int i = 0; i < simulateruns; i++){

        //NewtonUpdate(World, body, NEWTON_TIMESTEP);
        
        NewtonUpdate(World, NEWTON_TIMESTEP);
    }

    // Reset the update event //
    NewtonSetIslandUpdateEvent(World, NULL);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::ClearTimers(){
	LastSimulatedTime = Misc::GetTimeMicro64();
	PassedTimeTotal = 0;
}
// ------------------------------------ //
DLLEXPORT NewtonWorld* Leviathan::PhysicalWorld::GetNewtonWorld(){
	return World;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::AdjustClock(int milliseconds){

    // Convert from milliseconds (10^-3) to micro seconds (10^-6) //
    LastSimulatedTime -= 1000*milliseconds;
}
// ------------------------------------ //


