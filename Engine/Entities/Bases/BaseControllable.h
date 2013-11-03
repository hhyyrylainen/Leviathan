#ifndef LEVIATHAN_BASECONTROLLABLE
#define LEVIATHAN_BASECONTROLLABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseNotifiable.h"


namespace Leviathan{

	class BaseEntityController;

	// Entities that can be controlled should inherit this class //
	class BaseControllable : public BaseNotifiable{
	public:
		DLLEXPORT BaseControllable();
		DLLEXPORT ~BaseControllable();


	protected:


	};
}
#endif