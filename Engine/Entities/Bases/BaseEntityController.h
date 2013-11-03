#ifndef LEVIATHAN_BASEENTITYCONTROLLER
#define LEVIATHAN_BASEENTITYCONTROLLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseNotifier.h"
#include "BaseControllable.h"

namespace Leviathan{

	// Base class for all kinds of entity controllers to inherit from //
	class BaseEntityController : public BaseNotifier{
	public:
		DLLEXPORT BaseEntityController();
		DLLEXPORT ~BaseEntityController();



		// Callback which is called when rendering is about to occur, used for updating positions //
		DLLEXPORT virtual void UpdateControlledPositions(int mspassed) = 0;


	protected:



	};
}

#endif