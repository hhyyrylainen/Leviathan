#ifndef LEVIATHAN_ENTITY_LIGHT
#define LEVIATHAN_ENTITY_LIGHT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "..\Bases\BasePositionable.h"
#include "..\Bases\BaseObject.h"


namespace Leviathan{ namespace Entity{

	class Light : public BaseObject, public BasePositionable{
	public:
		DLLEXPORT Light();
		DLLEXPORT ~Light();


	protected:



	};

}}
#endif