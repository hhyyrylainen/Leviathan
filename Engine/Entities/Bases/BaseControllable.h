#ifndef LEVIATHAN_BASECONTROLLABLE
#define LEVIATHAN_BASECONTROLLABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseNotifiableEntity.h"


namespace Leviathan{


	//! Entities that can be controlled should inherit this class
	class BaseControllable : public BaseNotifiableEntity{
	public:
		DLLEXPORT BaseControllable();
		DLLEXPORT virtual ~BaseControllable();


	protected:


	};
}
#endif