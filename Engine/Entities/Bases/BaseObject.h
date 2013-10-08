#ifndef LEVIATHAN_BASEOBJECT
#define LEVIATHAN_BASEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class BaseObject{
	public:
		DLLEXPORT BaseObject(int id);
		DLLEXPORT virtual ~BaseObject();
		

		DLLEXPORT inline int GetID(){
			return ID;
		}


	protected:
		int ID;
	};

}
#endif