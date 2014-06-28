#ifndef LEVIATHAN_BASENOTIFIABLE
#define LEVIATHAN_BASENOTIFIABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ThreadSafe.h"

namespace Leviathan{

	//! This class is used to allow objects to connect to other objects safely 
	//! 
	//! This works by using unhook events that are called on both if either one is destroyed
	template<class ParentType, class ChildType>
	class BaseNotifiable : public virtual ThreadSafe{
	public:
		DLLEXPORT BaseNotifiable(ChildType* ourptr);
		DLLEXPORT virtual ~BaseNotifiable();

		//! \brief Release function which releases all hooks
		DLLEXPORT void ReleaseParentHooks();

		//! \brief The actual implementation of UnConnectFromNotifier
		DLLEXPORT bool UnConnectFromNotifier(BaseNotifier<ParentType, ChildType>* specificnotifier, ObjectLock &guard);

		//! \brief Notifies all the parents of this object about something
		//!
		//! This will call the BaseNotifier::OnNotified on all the child objects
		DLLEXPORT virtual void NotifyAll();

		//! \brief Disconnects this from a previously connected notifier
		DLLEXPORT FORCE_INLINE bool UnConnectFromNotifier(BaseNotifier<ParentType, ChildType>* specificnotifier){
			// The parent has to be locked before this object //
			GUARD_LOCK_OTHER_OBJECT_NAME(specificnotifier, guard2);
			GUARD_LOCK_THIS_OBJECT();
			return UnConnectFromNotifier(specificnotifier, guard);
		}

		//! \brief This searches the connected notifiers and calls the above function with it's pointer
		DLLEXPORT bool UnConnectFromNotifier(int id);

		//! \brief Connects this to a notifier object calling all the needed functions
		//! \todo return false and skip adding if already added
		DLLEXPORT bool ConnectToNotifier(BaseNotifier<ParentType, ChildType>* owner);

		//! Callback called by the parent, used to not to call the unhook again on the parent
		void _OnUnhookNotifier(BaseNotifier<ParentType, ChildType>* parent);

		//! Called by parent to hook, and doesn't call the parent's functions
		void _OnHookNotifier(BaseNotifier<ParentType, ChildType>* parent);

		//! \brief Gets the internal pointer to the actual object
		DLLEXPORT ChildType* GetActualPointerToNotifiableObject();

		//! \brief Called when our parent notifies us about something
		//! \note Both the parent and this object has been locked when this is called
		//! \warning Do not directly call this if you don't know what you are doing!
		virtual void OnNotified();


	protected:

		// Callbacks for child classes to implement //
		DLLEXPORT virtual void _OnNotifierConnected(ParentType* parentadded);
		DLLEXPORT virtual void _OnNotifierDisconnected(ParentType* parenttoremove);
		// ------------------------------------ //

		//! Stores a pointer to the object that is inherited from this
		ChildType* PointerToOurNotifiable;

		//! Vector of other objects that this is connected to
		std::vector<BaseNotifier<ParentType, ChildType>*> ConnectedToParents;

		//! This lock is used to keep locked while an operation needs this object to stay 
	};


	//! \brief Specialized class for accepting all parent/child objects
	class BaseNotifiableAll : public BaseNotifiable<BaseNotifierAll, BaseNotifiableAll>{
	public:
		DLLEXPORT inline BaseNotifiableAll() : BaseNotifiable(this){
		}
		DLLEXPORT inline ~BaseNotifiableAll(){
		}
	};
}

// The implementations are included here to make this compile //
#include "BaseNotifiableImpl.h"

#endif