#ifndef LEVIATHAN_BASEOBJECT
#define LEVIATHAN_BASEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


enum ENTITYCUSTOMMESSAGETYPE {ENTITYCUSTOMMESSAGETYPE_LOCATIONDATA_UPDATED, ENTITYCUSTOMMESSAGETYPE_POSITIONUPDATED, 
	ENTITYCUSTOMMESSAGETYPE_ORIENTATIONUPDATED};

namespace Leviathan{

	class GameWorld;

	class BaseObject{
	public:
		DLLEXPORT BaseObject(int id, GameWorld* worldptr);
		DLLEXPORT virtual ~BaseObject();
		
		// Called before deletion and should release objects that need to be deleted during world release phase (like graphical nodes) //
		DLLEXPORT virtual void Release();

		DLLEXPORT inline int GetID(){
			return ID;
		}
		DLLEXPORT inline GameWorld* GetWorld(){
			return LinkedToWorld;
		}

		// This function is used to avoid explicit dynamic casts when trying to call features on entities that they might not have //
		// Should return true if the message is acknowledged so that the caller can avoid calling more general types //
		DLLEXPORT virtual bool SendCustomMessage(ENTITYCUSTOMMESSAGETYPE type, void* dataptr) = 0;


	protected:
		int ID;
		// All objects should be in some world (even if not really in a world, then a dummy world) //
		GameWorld* LinkedToWorld;
	};

}
#endif