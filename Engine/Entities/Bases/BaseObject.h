#ifndef LEVIATHAN_BASEOBJECT
#define LEVIATHAN_BASEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


// This cannot be enum because it need to be able to be extended with new values //
#define ENTITYCUSTOMMESSAGETYPE_LOCATIONDATA_UPDATED			1
#define ENTITYCUSTOMMESSAGETYPE_POSITIONUPDATED					2
#define ENTITYCUSTOMMESSAGETYPE_ORIENTATIONUPDATED				3

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
		DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr) = 0;


	protected:
		int ID;
		// All objects should be in some world (even if not really in a world, then a dummy world) //
		GameWorld* LinkedToWorld;
	};

}
#endif