#ifndef LEVIATHAN_BASEENTITYCONTROLLER
#define LEVIATHAN_BASEENTITYCONTROLLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseControllable.h"
#include "BaseConstraintable.h"

namespace Leviathan{

	// Base class for all kinds of entity controllers to inherit from //
	class BaseEntityController : public BaseConstraintable, virtual public BaseObject{
	public:
		DLLEXPORT BaseEntityController();
		DLLEXPORT ~BaseEntityController();



		// Callback which is called when physics update is about to occur, used for updating positions //
		DLLEXPORT virtual void UpdateControlledPositions(float timestep) = 0;


	protected:



	};
}

#endif
