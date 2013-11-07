#ifndef LEVIATHAN_PHYSICALWORLD
#define LEVIATHAN_PHYSICALWORLD
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <Newton.h>

#define NEWTON_DEFAULT_PHYSICS_FPS		150.f
#define NEWTON_FPS_IN_MICROSECONDS		(1000000.0f/NEWTON_DEFAULT_PHYSICS_FPS)
#define NEWTON_TIMESTEP					(NEWTON_FPS_IN_MICROSECONDS/1000000.0f)


namespace Leviathan{

	class GameWorld;

	class PhysicalWorld : public Object{
	public:
		// The constructor also builds the material list for the world, so it is rather expensive //
		DLLEXPORT PhysicalWorld(GameWorld* owner);
		DLLEXPORT ~PhysicalWorld();

		DLLEXPORT void SimulateWorld();
		DLLEXPORT void ClearTimers();

		DLLEXPORT inline NewtonWorld* GetWorld(){
			return World;
		}

		DLLEXPORT NewtonWorld* GetNewtonWorld();

	protected:

		__int64 LastSimulatedTime;
		int PassedTimeTotal;

		NewtonWorld* World;
		GameWorld* OwningWorld;
	};

}
#endif