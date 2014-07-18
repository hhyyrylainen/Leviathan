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
	class BaseNotifier : public virtual ThreadSafe{
	public:
		DLLEXPORT BaseNotifier(ParentType* ourptr);
		DLLEXPORT virtual ~BaseNotifier();

		//! Release function that unhooks all child objects
		DLLEXPORT void ReleaseChildHooks();

		//! Connects this to a notifiable object for holding a reference to it
		DLLEXPORT FORCE_INLINE bool ConnectToNotifiable(BaseNotifiable<ParentType, ChildType>* child){
			GUARD_LOCK_THIS_OBJECT();
			return ConnectToNotifiable(child, guard);
		}

		//! \brief The actual implementation of ConnecToNotifiable
		DLLEXPORT bool ConnectToNotifiable(BaseNotifiable<ParentType, ChildType>* child, ObjectLock &guard);


		//! \brief Notifies the children about something
		//!
		//! This will call the BaseNotifiable::OnNotified on all the child objects
		DLLEXPORT virtual void NotifyAll();


		//! \brief Disconnects from previously connected notifiable
		DLLEXPORT bool UnConnectFromNotifiable(BaseNotifiable<ParentType, ChildType>* unhookfrom, ObjectLock &guard);

		DLLEXPORT FORCE_INLINE bool UnConnectFromNotifiable(BaseNotifiable<ParentType, ChildType>* child){
			GUARD_LOCK_THIS_OBJECT();
			return UnConnectFromNotifiable(child, guard);
		}

		//! \brief Searches the connected notifiable objects and calls the above function with it's pointer
		DLLEXPORT bool UnConnectFromNotifiable(int id);

		//! \brief Actual implementation of this method
		DLLEXPORT bool IsConnectedTo(BaseNotifiable<ParentType, ChildType>* check, ObjectLock &guard);

		//! \brief Returns true when the specified object is already connected
		DLLEXPORT FORCE_INLINE bool IsConnectedTo(BaseNotifiable<ParentType, ChildType>* check){
			GUARD_LOCK_THIS_OBJECT();
			IsConnectedTo(check, guard);
		}
		
		// Callback called by the child, and doesn't call the unhook again on the child
		void _OnUnhookNotifiable(BaseNotifiable<ParentType, ChildType>* childtoremove);

		// Called by child to hook, and doesn't call the child's functions
		void _OnHookNotifiable(BaseNotifiable<ParentType, ChildType>* child);

		//! \brief Gets the internal pointer to the actual object
		DLLEXPORT ParentType* GetActualPointerToNotifierObject();

		//! \brief Called when one of our children notifies us about something
		//! \note Both the child and this object has been locked when this is called
		//! \warning Do not directly call this if you don't know what you are doing!
		virtual void OnNotified();


	protected:

		// Callbacks for child classes to implement //
		// This object should already be locked during this call //
		DLLEXPORT virtual void _OnNotifiableConnected(ChildType* childadded);
		DLLEXPORT virtual void _OnNotifiableDisconnected(ChildType* childtoremove);
		// ------------------------------------ //

		//! Stores a pointer to the object that is inherited from this
		ParentType* PointerToOurNotifier;

		//! Vector of other objects that this is connected to
		std::vector<BaseNotifiable<ParentType, ChildType>*> ConnectedChildren;
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

// The implementations are included here to make this compile //
#include "BaseNotifierImpl.h"

#endif