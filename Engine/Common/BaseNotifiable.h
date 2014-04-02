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
	class BaseNotifiable : public ThreadSafe{
	public:
		DLLEXPORT BaseNotifiable(ChildType* ourptr);
		DLLEXPORT virtual ~BaseNotifiable();

		//! \brief Release function which releases all hooks
		DLLEXPORT void ReleaseParentHooks();

		//! \brief The actual implementation of UnConnectFromNotifier
		DLLEXPORT bool UnConnectFromNotifier(BaseNotifier<ParentType, ChildType>* specificnotifier, ObjectLock &guard);

		//! \brief Disconnects this from a previously connected notifier
		DLLEXPORT FORCE_INLINE bool UnConnectFromNotifier(BaseNotifier<ParentType, ChildType>* specificnotifier){
			GUARD_LOCK_THIS_OBJECT();
			return UnConnectFromNotifier(specificnotifier, guard);
		}
		//! \brief This searches the connected notifiers and calls the above function with it's pointer
		DLLEXPORT bool UnConnectFromNotifier(int id);

		//! \brief Connects this to a notifier object calling all the needed functions
		DLLEXPORT bool ConnectToNotifier(BaseNotifier<ParentType, ChildType>* owner);


		//! Callback called by the parent, used to not to call the unhook again on the parent
		void _OnUnhookNotifier(BaseNotifier<ParentType, ChildType>* parent);

		//! Called by parent to hook, and doesn't call the parent's functions
		void _OnHookNotifier(BaseNotifier<ParentType, ChildType>* parent);

		//! \brief Gets the internal pointer to the actual object
		DLLEXPORT ChildType* GetActualPointerToNotifiableObject();

	protected:

		// Callbacks for child classes to implement //
		DLLEXPORT virtual void _OnNotifierConnected(ParentType* parentadded);
		DLLEXPORT virtual void _OnNotifierDisconnected(ParentType* parenttoremove);
		// ------------------------------------ //

		//! Stores a pointer to the object that is inherited from this
		ChildType* PointerToOurNotifiable;

		//! Vector of other objects that this is connected to
		std::vector<BaseNotifier<ParentType, ChildType>*> ConnectedToParents;
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