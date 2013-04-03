#ifndef LEVIATHAN_BASEOBJECT
#define LEVIATHAN_BASEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "MultiFlag.h"

#define OBJECT_TYPE_BASE 1


#define OBJECT_TYPE_FLYING_CAMERA	1000
#define OBJECT_TYPE_MODEL			20000

namespace Leviathan{

	class BaseObject /*: public Object these classes are "components" and shouldn't inherit anything */{
	public:
		DLLEXPORT BaseObject::BaseObject();
		DLLEXPORT virtual BaseObject::~BaseObject();
		

		int ID;
		int Type;
		//bool higher;

		MultiFlag* Flags;
	private:

	};

}
#endif