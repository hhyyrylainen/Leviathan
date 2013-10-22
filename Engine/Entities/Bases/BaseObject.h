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
		
		// Called before deletion and should release objects that need to be deleted during world release phase (like graphical nodes) //
		DLLEXPORT virtual void Release();

		DLLEXPORT inline int GetID(){
			return ID;
		}


	protected:
		int ID;
	};

}
#endif