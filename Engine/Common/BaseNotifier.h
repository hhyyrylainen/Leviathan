#ifndef LEVIATHAN_BASENOTIFIER
#define LEVIATHAN_BASENOTIFIER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ThreadSafe.h"


namespace Leviathan{

	template<class ParentType, class ChildType>
	class BaseNotifier : public ThreadSafe{
	public:
		DLLEXPORT BaseNotifier(ParentType* ourptr);
		DLLEXPORT virtual ~BaseNotifier();

		//! Release function that unhooks all child objects
		DLLEXPORT void ReleaseChildHooks();

		//! Connects this to a notifiable object for holding a reference to it //
		//! \todo return false and skip adding if already added
		DLLEXPORT bool ConnectToNotifiable(BaseNotifiable<ParentType, ChildType>* child);
		//! Disconnects from previously connected notifiable //
		DLLEXPORT bool UnConnectFromNotifiable(BaseNotifiable<ParentType, ChildType>* unhookfrom);
		//! This searches the connected notifiable objects and calls the above function with it's pointer //
		DLLEXPORT bool UnConnectFromNotifiable(int id);


		// Callback called by the child, and doesn't call the unhook again on the child //
		void _OnUnhookNotifiable(BaseNotifiable<ParentType, ChildType>* childtoremove);

		// Called by child to hook, and doesn't call the child's functions //
		void _OnHookNotifiable(BaseNotifiable<ParentType, ChildType>* child);

		//! \brief Gets the internal pointer to the actual object
		DLLEXPORT ParentType* GetActualPointerToNotifierObject();

	protected:

		// Callbacks for child classes to implement //
		DLLEXPORT virtual void _OnNotifiableConnected(ChildType* childadded);
		DLLEXPORT virtual void _OnNotifiableDisconnected(ChildType* childtoremove);
		// ------------------------------------ //

		//! Stores a pointer to the object that is inherited from this
		ParentType* PointerToOurNotifier;

		//! List of other objects that this is connected to
		//! \todo Check if list is actually better than a vector
		std::list<BaseNotifiable<ParentType, ChildType>*> ConnectedChildren;
	};

	//! \brief Specialized class for accepting all parent/child objects
	class BaseNotifierAll : public BaseNotifier<BaseNotifierAll, BaseNotifiableAll>{
	public:
		DLLEXPORT inline BaseNotifierAll() : BaseNotifier(this){
		}
		DLLEXPORT inline ~BaseNotifierAll(){
		}
	};

}
#endif