#ifndef LEVIATHAN_BASEPARENTABLE
#define LEVIATHAN_BASEPARENTABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseNotifiable.h"
#include "BaseNotifier.h"
#include "BasePositionable.h"


#define BASEPARENTABLE_CUSTOMMESSAGE_DATA_CHECK		{if(entitycustommessagetype >= ENTITYCUSTOMMESSAGETYPE_PARENTPOSITIONUPDATED && entitycustommessagetype <= ENTITYCUSTOMMESSAGETYPE_PARENTCONNECTED){if(BaseParentableCustomMessage(entitycustommessagetype, dataptr)) return true;}}
#define BASEPARENTABLE_CUSTOMMESSAGE_GET_CHECK		{if(false){if(BaseParentableCustomGetData(tmprequest)) return true;}}


namespace Leviathan{

	// This class can be inherited to be able to have non-physics controlled child objects and be parentable to other objects //
	// TODO: create physical version of this class using constraints
	class BaseParentable : public BaseNotifiable, public BaseNotifier, virtual public BasePositionable{
	public:
		DLLEXPORT BaseParentable();
		DLLEXPORT virtual ~BaseParentable();

		// TODO: implement circular reference check to avoid stack overflows //
		// TODO: add onunconnect virtual function

		DLLEXPORT bool AddNonPhysicsChild(BaseParentable* childobject);
		DLLEXPORT bool AddNonPhysicsParent(BaseParentable* parentobject);

		// TODO: add remove functionality (while this is unimplemented child classes can directly call BaseNotifiable and notifier functions to get
		// rid of parent objects)

	protected:
		// This needs to be called every time the object's position (or orientation) changes //
		void _ParentableNotifyLocationDataUpdated();

		// This is called when the position has been adjusted by the parent (will be called before other notification functions) //
		virtual void _OnParentablePositionUpdated();

		bool BaseParentableCustomMessage(int message, void* data);
		bool BaseParentableCustomGetData(ObjectDataRequest* data);

		// ------------------------------------ //
		// Used to store data about the parent. This is used to adjust the position of this object if parent moves (eliminates the need to have extra data
		// in derived classes to add parent position to real position
		Float3 ParentOldPos;
		// Not exactly sure how to apply this, but it could possibly be done like this:
		// Get the new rotation and create a temporary value like this (ParentOldRot-NewRot).Normalize() and multiply the current value
		// Orientation = Orientation*temp
		Float4 ParentOldRot;

	};

}
#endif