#ifndef LEVIATHAN_BASEENTITYCONTROLLER
#define LEVIATHAN_BASEENTITYCONTROLLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseControllable.h"
#include "BaseNotifierEntity.h"

namespace Leviathan{

	// Base class for all kinds of entity controllers to inherit from //
	class BaseEntityController : public BaseNotifierEntity{
	public:
		DLLEXPORT BaseEntityController();
		DLLEXPORT ~BaseEntityController();



		// Callback which is called when physics update is about to occur, used for updating positions //
		DLLEXPORT virtual void UpdateControlledPositions(float timestep) = 0;


	protected:



	};
}

#endif