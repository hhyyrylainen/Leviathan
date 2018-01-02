// ------------------------------------ //
#include "PhysicalWorld.h"

#include <Newton.h>
#include "PhysicsMaterialManager.h"
#include "Events/EventHandler.h"
#include "../TimeIncludes.h"
#include "Engine.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::PhysicalWorld::PhysicalWorld(GameWorld* owner) :
    OwningWorld(owner)
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

    // Cap passed time, if over one second //
    if(PassedTimeTotal > MICROSECONDS_IN_SECOND)
        PassedTimeTotal = MICROSECONDS_IN_SECOND;
    
    Lock lock(WorldUpdateLock);

	while(PassedTimeTotal >= NEWTON_FPS_IN_MICROSECONDS){
        
		// Call event //
        Engine::Get()->GetEventHandler()->CallEvent(new Event(EVENT_TYPE_PHYSICS_BEGIN,
                new PhysicsStartEventData(NEWTON_TIMESTEP,
                    OwningWorld)));

		NewtonUpdate(World, NEWTON_TIMESTEP);
		PassedTimeTotal -= static_cast<int64_t>(NEWTON_FPS_IN_MICROSECONDS);
        runs++;

        if(runs == maxruns){

            Logger::Get()->Warning("PhysicalWorld: bailing from update after " +
                Convert::ToString(runs) + " with time left: " +
                Convert::ToString(PassedTimeTotal));
            break;
        }
	}
}

DLLEXPORT void Leviathan::PhysicalWorld::SimulateWorldFixed(uint32_t mspassed, 
    uint32_t stepcount /*= 1*/) 
{
    float timestep = (mspassed / 1000.f) / stepcount;

    for (uint32_t i = 0; i < stepcount; ++i) {

        Engine::Get()->GetEventHandler()->CallEvent(new Event(EVENT_TYPE_PHYSICS_BEGIN,
            new PhysicsStartEventData(timestep,
                OwningWorld)));

        NewtonUpdate(World, timestep);
    }
}
// ------------------------------------ //
int Leviathan::SingleBodyUpdate(const NewtonWorld* const newtonWorld, const void* islandHandle,
    int bodyCount)
{

    PhysicalWorld* pworld = reinterpret_cast<PhysicalWorld*>(
        NewtonWorldGetUserData(newtonWorld));
    
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



