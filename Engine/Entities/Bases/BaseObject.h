#ifndef LEVIATHAN_BASEOBJECT
#define LEVIATHAN_BASEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


// This cannot be enum because it need to be able to be extended with new values //
// Following rules should be followed:
// Internal engine messages have reserved numbers below 10000
// And direct to script calls should be over 100000 (100 thousand) for easy checking by callee //

#define ENTITYCUSTOMMESSAGETYPE_LOCATIONDATA_UPDATED			1
#define ENTITYCUSTOMMESSAGETYPE_POSITIONUPDATED					2
#define ENTITYCUSTOMMESSAGETYPE_ORIENTATIONUPDATED				3

// For this type ptr should be ApplyForceInfo* which should be deleted BY the CALLER (don't store this, just copy values out) //
#define ENTITYCUSTOMMESSAGETYPE_ADDAPPLYFORCE					4
// For this type ptr should be wstring* which doesn't need deleting //
#define ENTITYCUSTOMMESSAGETYPE_REMOVEAPPLYFORCE				5
// For this type ptr should be Float3* which doesn't need deleting //
#define ENTITYCUSTOMMESSAGETYPE_SETVELOCITY						6
// For this type ptr should be Float3* which doesn't need deleting //
#define ENTITYCUSTOMMESSAGETYPE_CHANGEWORLDPOSITION				7
// ------------------ These are sent by parent objects (only non-physical parenting) ------------------ //
// Sent when the position of the parent object has updated, for this type the ptr should be BasePositionable* which doesn't need deleting //
#define ENTITYCUSTOMMESSAGETYPE_PARENTPOSITIONUPDATED			8
// Sent when parent has connected (won't be send when child connects to a parent), for this type ptr should be BasePositionable* which doesn't need deleting //
#define ENTITYCUSTOMMESSAGETYPE_PARENTCONNECTED					9

// This message type is used for requesting data, if cannot be handled just return false, but if it can return the right type in the void ptr //
#define ENTITYCUSTOMMESSAGETYPE_DATAREQUEST						9999

namespace Leviathan{

	struct ObjectDataRequest{
		ObjectDataRequest(int wantedtype) : RequestObjectPart(wantedtype), RequestResult(NULL){
		}

		// See the following bunch of defines for values //
		int RequestObjectPart;
		void* RequestResult;
		// Used to send more info like another object //
		void* AdditionalInfo;
	};
}
// Same rules should apply to these than the defines above (don't use values less than 10000) //

// Expected result is Float3*, additional is NULL //
#define ENTITYDATA_REQUESTTYPE_WORLDPOSITION		1


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
			return OwnedByWorld;
		}

		// This function is used to avoid explicit dynamic casts when trying to call features on entities that they might not have //
		// Should return true if the message is acknowledged so that the caller can avoid calling more general types //
		DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr) = 0;


	protected:
		int ID;
		// All objects should be in some world (even if not really in a world, then a dummy world) //
		GameWorld* OwnedByWorld;
	};

}
#endif