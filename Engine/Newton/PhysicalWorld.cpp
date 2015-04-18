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

    __int64 curtime = Misc::GetTimeMicro64();
    
	// Calculate passed time and reset //
	PassedTimeTotal += curtime-LastSimulatedTime;
	LastSimulatedTime = curtime;

    
    boost::unique_lock<boost::mutex> lock(WorldUpdateLock);

	while(PassedTimeTotal >= NEWTON_FPS_IN_MICROSECONDS){
        
		// Call event //
		EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_PHYSICS_BEGIN, new PhysicsStartEventData(NEWTON_TIMESTEP,
                    OwningWorld)));

		NewtonUpdate(World, NEWTON_TIMESTEP);
		PassedTimeTotal -= NEWTON_FPS_IN_MICROSECONDS;
        runs++;

        if(runs == maxruns){

            Logger::Get()->Warning("PhysicalWorld: bailing from update after "+Convert::ToString(runs)+
                " with time left: "+Convert::ToString(PassedTimeTotal));
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

DLLEXPORT void Leviathan::PhysicalWorld::ResimulateBody(NewtonBody* body, int milliseconds,
    boost::function<void()> callback, BaseConstraintable* targetentity)
{
    
    boost::unique_lock<boost::mutex> lock(WorldUpdateLock);

    ResimulatedBody = body;

    // Setup single island callbacks //
    NewtonSetIslandUpdateEvent(World, &SingleBodyUpdate);

    int64_t passedtime = milliseconds*1000;
    
    while(passedtime >= NEWTON_FPS_IN_MICROSECONDS){

        if(targetentity)
            EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_PHYSICS_RESIMULATE_SINGLE,
                    new ResimulateSingleEventData(passedtime, targetentity, OwningWorld)));

		NewtonUpdate(World, NEWTON_TIMESTEP);
        passedtime-= NEWTON_FPS_IN_MICROSECONDS;

        callback();
	}
    
#ifdef ALLOW_RESIMULATE_CONSUME_ALL
    
    // Update away any left over time //
    if(passedtime > 0){

        // Might be a bad idea to use a varying time step here...
        NewtonUpdate(World, passedtime/1000000.f);
    }
    
#endif //ALLOW_RESIMULATE_CONSUME_ALL
    
    // Reset the update event //
    NewtonSetIslandUpdateEvent(World, NULL);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::ClearTimers(){
	LastSimulatedTime = Misc::GetTimeMicro64();
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





