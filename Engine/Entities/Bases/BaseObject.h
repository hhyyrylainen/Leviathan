#ifndef LEVIATHAN_BASEOBJECT
#define LEVIATHAN_BASEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


#define OBJECT_TYPE_BASE				1
#define OBJECT_TYPE_FLYING_CAMERA		1000
#define OBJECT_TYPE_MODEL				20000


namespace Leviathan{

	class BaseObject{
	public:
		DLLEXPORT BaseObject::BaseObject();
		DLLEXPORT virtual BaseObject::~BaseObject();
		

		int ID;
		int Type;


	private:

	};

}
#endif