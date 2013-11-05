#ifndef LEVIATHAN_BASENOTIFIER
#define LEVIATHAN_BASENOTIFIER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseNotifiable.h"
#include "BaseObject.h"

namespace Leviathan{

	class BaseNotifier : virtual public BaseObject{
		friend BaseNotifiable;
	public:
		DLLEXPORT BaseNotifier();
		DLLEXPORT virtual ~BaseNotifier();

		// Release function that unhooks all child objects //
		DLLEXPORT void ReleaseChildHooks();

		// Connects this to a notifiable object for holding a reference to it //
		DLLEXPORT bool ConnectToNotifiable(BaseNotifiable* child);
		// Disconnects from previously connected notifiable //
		DLLEXPORT bool UnConnectFromNotifiable(BaseNotifiable* unhookfrom);
		// This searches the connected notifiable objects and calls the above function with it's pointer //
		DLLEXPORT bool UnConnectFromNotifiable(int id);


	protected:

		// Callback called by the child, and doesn't call the unhook again on the child //
		void _OnUnhookNotifiable(BaseNotifiable* childtoremove);
		// Called by child to hook, and doesn't call the child's functions //
		void _OnHookNotifiable(BaseNotifiable* child);

		// Callbacks for child classes to implement //
		virtual void _OnNotifiableConnected(BaseNotifiable* childadded);
		virtual void _OnNotifiableDisconnected(BaseNotifiable* childtoremove);
		// ------------------------------------ //

		std::list<BaseNotifiable*> ConnectedChilds;
	};

}
#endif