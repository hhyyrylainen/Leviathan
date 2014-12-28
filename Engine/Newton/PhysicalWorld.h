#ifndef LEVIATHAN_PHYSICALWORLD
#define LEVIATHAN_PHYSICALWORLD
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
//#ifdef __GNUC__
// Stop newton warnings from popping up
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wextern-c-compat"
//#endif //__GNUC__

#include <Newton.h>
#include "boost/thread/mutex.hpp"

//#ifdef __GNUC__
//#pragma GCC diagnostic pop
//#endif //__GNUC__

#define NEWTON_DEFAULT_PHYSICS_FPS		150.f
#define NEWTON_FPS_IN_MILLISECONDS		(1000.0f/NEWTON_DEFAULT_PHYSICS_FPS)
#define NEWTON_TIMESTEP					(NEWTON_FPS_IN_MILLISECONDS/1000.0f)


namespace Leviathan{

    int SingleBodyUpdate(const NewtonWorld* const newtonWorld, const void* islandHandle, int bodyCount);
    
	class PhysicalWorld : public Object{
        friend int SingleBodyUpdate(const NewtonWorld* const newtonWorld, const void*
            islandHandle, int bodyCount);
	public:
		// The constructor also builds the material list for the world, so it is rather expensive //
		DLLEXPORT PhysicalWorld(GameWorld* owner);
		DLLEXPORT ~PhysicalWorld();

        //! \brief Add passed time to be simulated away
        DLLEXPORT void AccumulateTime(int milliseconds);
        
        //! \todo Improve performance by making this use a newton method that simulates a single body
        //! instead of doing a full update and just discarding all changing bodies that aren't wanted
        //! \todo Add an event that allows entity controllers to update forces when one of their controlled
        //! entities are being simulated
        DLLEXPORT void ResimulateBody(NewtonBody* body, int milliseconds);
        
        //! \brief Simulates away all accumulated time
        DLLEXPORT void ConsumeTime(int maxruns = -1);


        DLLEXPORT void ResetPassedTime();

        //! \todo Make this return a newton lock that needs to be held while the pointer is used
		DLLEXPORT NewtonWorld* GetNewtonWorld();

	protected:

        //! Total amount of milliseconds required to be simulated
		float PassedTimeTotal;

		NewtonWorld* World;
		GameWorld* OwningWorld;

        //! Lock for world updates
        boost::mutex WorldUpdateLock;

        //! Used for resimulation
        //! \todo Potentially allow this to be a vector
        NewtonBody* ResimulatedBody;
	};

}
#endif
