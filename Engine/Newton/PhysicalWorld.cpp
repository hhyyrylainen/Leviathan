// ------------------------------------ //
#include "PhysicalWorld.h"

#include <Newton.h>
#include "PhysicsMaterialManager.h"
#include "Events/EventHandler.h"
#include "../TimeIncludes.h"
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

    auto physmanager = PhysicsMaterialManager::Get();
    
    if(physmanager)
        physmanager->DestroyActualMaterialsForWorld(World);

    World = NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::SimulateWorld(int maxruns /*= -1*/){

    if(maxruns == 0){

        // TODO: report error?
        maxruns = 1;
    }
    
    int runs = 0;

    auto curtime = Time::GetTimeMicro64();
    
	// Calculate passed time and reset //
	PassedTimeTotal += curtime-LastSimulatedTime;
	LastSimulatedTime = curtime;

    
    Lock lock(WorldUpdateLock);

	while(PassedTimeTotal >= NEWTON_FPS_IN_MICROSECONDS){
        
		// Call event //
		EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_PHYSICS_BEGIN,
                new PhysicsStartEventData(NEWTON_TIMESTEP,
                    OwningWorld)));

		NewtonUpdate(World, NEWTON_TIMESTEP);
		PassedTimeTotal -= NEWTON_FPS_IN_MICROSECONDS;
        runs++;

        if(runs == maxruns){

            Logger::Get()->Warning("PhysicalWorld: bailing from update after "+
                Convert::ToString(runs)+" with time left: "+Convert::ToString(PassedTimeTotal));
            return;
        }
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
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::ClearTimers(){
	LastSimulatedTime = Time::GetTimeMicro64();
	PassedTimeTotal = 0;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::AdjustClock(int milliseconds){

    // Convert from milliseconds (10^-3) to micro seconds (10^-6) //
    LastSimulatedTime -= 1000*milliseconds;
}
// ------------------------------------ //
DLLEXPORT NewtonWorld* Leviathan::PhysicalWorld::GetNewtonWorld(){
	return World;
}
// ------------------------------------ //



