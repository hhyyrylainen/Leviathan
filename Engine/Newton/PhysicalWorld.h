#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
// ---- includes ---- //
//#ifdef __GNUC__
// Stop newton warnings from popping up
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wextern-c-compat"
//#endif //__GNUC__

#include <Newton.h>
#include <functional>
#include "../Common/ThreadSafe.h"

//#ifdef __GNUC__
//#pragma GCC diagnostic pop
//#endif //__GNUC__

#define NEWTON_DEFAULT_PHYSICS_FPS		150.f
#define NEWTON_FPS_IN_MICROSECONDS		(1000000.0f/NEWTON_DEFAULT_PHYSICS_FPS)
#define NEWTON_TIMESTEP					(NEWTON_FPS_IN_MICROSECONDS/1000000.0f)


namespace Leviathan{

    int SingleBodyUpdate(const NewtonWorld* const newtonWorld, const void* islandHandle, int bodyCount);
    
	class PhysicalWorld{
        friend int SingleBodyUpdate(const NewtonWorld* const newtonWorld, const void*
            islandHandle, int bodyCount);
	public:
		// The constructor also builds the material list for the world, so it is rather expensive //
		DLLEXPORT PhysicalWorld(GameWorld* owner);
		DLLEXPORT ~PhysicalWorld();

        //! \brief Calculates and simulates away all accumulated time
        DLLEXPORT void SimulateWorld(int maxruns = -1);

        //! \brief Clears passed time
		DLLEXPORT void ClearTimers();

        //! \brief Adds or subtracts time from the clock
        //!
        //! For example passing in 100 will run the physical simulation more times next update
        //! to account for milliseconds amount of passed time
        DLLEXPORT void AdjustClock(int milliseconds);


        //! \todo Make this return a newton lock that needs to be held while the pointer is used
		DLLEXPORT NewtonWorld* GetNewtonWorld();

	protected:

        //! Total amount of microseconds required to be simulated
		int64_t PassedTimeTotal;
        int64_t LastSimulatedTime;

		NewtonWorld* World;
		GameWorld* OwningWorld;

        //! Lock for world updates
        Mutex WorldUpdateLock;

        //! Used for resimulation
        //! \todo Potentially allow this to be a vector
        NewtonBody* ResimulatedBody;
	};

}

