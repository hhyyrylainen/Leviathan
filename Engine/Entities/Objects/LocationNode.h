#ifndef LEVIATHAN_LOCATIONNODEENTITY
#define LEVIATHAN_LOCATIONNODEENTITY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "..\Bases\BasePositionable.h"
#include "..\Bases\BaseNotifiable.h"


namespace Leviathan{ namespace Entity{

	// Class that is used for representing positions and orientations //
	// This class is child notifier because it is meant to be included in other objects and not the other way around //
	class LocationNode : public BasePositionable, public BaseNotifiable, virtual public BaseObject{
	public:
		// Positions at origin and uses identity rotation //
		DLLEXPORT LocationNode(GameWorld* world, bool deleteifnoowner = true);
		// Uses provided position and rotation //
		DLLEXPORT LocationNode(GameWorld* world, const Float3 &pos, const Float4 &orientation, bool deleteifnoowner = true);
		DLLEXPORT ~LocationNode();


	protected:
		// Function which sends a message to update the parent object that position of this object has changed //
		void _NotifyParentOfPosition();

		virtual void PosUpdated();
		virtual void OrientationUpdated();
		// Potentially deletes this object if flags are right and no owner is connected //
		virtual void _OnNotifierDisconnected(BaseNotifier* parenttoremove);
		// ------------------------------------ //

		// Dictates whether this node should delete itself without any parents attached //
		bool DeleteIfNoParent;

	};

}}
#endif