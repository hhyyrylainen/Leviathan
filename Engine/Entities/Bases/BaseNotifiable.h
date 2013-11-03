#ifndef LEVIATHAN_BASENOTIFIABLE
#define LEVIATHAN_BASENOTIFIABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseObject.h"


namespace Leviathan{

	// Forward declaration of the parent object to be able to include this in that file //
	class BaseNotifier;

	// This class is used to allow entities to connect to other entities safely (unhook events are called on both if either is destroyed) //
	class BaseNotifiable : virtual public BaseObject{
		friend BaseNotifier;
	public:
		DLLEXPORT BaseNotifiable();
		DLLEXPORT virtual ~BaseNotifiable();

		// Release function which releases all hooks //
		DLLEXPORT void ReleaseParentHooks();

		// Connects this to a notifier object calling all needed functions //
		DLLEXPORT bool ConnectToNotifier(BaseNotifier* owner);
		// Disconnects from previously connected notifier //
		DLLEXPORT bool UnConnectFromNotifier(BaseNotifier* specificnotifier);
		// This searches the connected notifiers and calls the above function with it's pointer //
		DLLEXPORT bool UnConnectFromNotifier(int id);

	protected:

		// Callback called by the parent, used to not to call the unhook again on the parent //
		void _OnUnhookNotifier(BaseNotifier* parent);
		// Called by parent to hook, and doesn't call the parent's functions //
		void _OnHookNotifier(BaseNotifier* parent);

		// Callbacks for child classes to implement //
		virtual void _OnNotifierConnected(BaseNotifier* parentadded);
		virtual void _OnNotifierDisconnected(BaseNotifier* parenttoremove);
		// ------------------------------------ //

		std::list<BaseNotifier*> ConnectedToParents;
	};

}
#endif