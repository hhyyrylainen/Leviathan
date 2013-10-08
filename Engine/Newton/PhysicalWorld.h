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

	class PhysicalWorld : public Object{
	public:
		DLLEXPORT PhysicalWorld();
		DLLEXPORT ~PhysicalWorld();

		DLLEXPORT void SimulateWorld();
		DLLEXPORT void ClearTimers();

		DLLEXPORT inline NewtonWorld* GetWorld(){
			return World;
		}

	protected:

		__int64 LastSimulatedTime;
		int PassedTimeTotal;

		NewtonWorld* World;
	};

}
#endif