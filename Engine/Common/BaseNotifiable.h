#ifndef BASENOTIFIABLE_H
#define BASENOTIFIABLE_H
#include "Define.h"
// ------------------------------------ //
#include "ThreadSafe.h"
#include <vector>

namespace Leviathan{

//! This class is used to allow objects to connect to other objects safely 
//! 
//! This works by using unhook events that are called on both if either one is destroyed
template<class ParentType, class ChildType>
	class BaseNotifiable : public virtual ThreadSafe{
public:
    BaseNotifiable(ChildType* ourptr);
    virtual ~BaseNotifiable();

    //! \brief Release function which releases all hooks
    void ReleaseParentHooks(Lock &guard);

    //! \brief The actual implementation of UnConnectFromNotifier
    bool UnConnectFromNotifier(Lock &guard,
        BaseNotifier<ParentType, ChildType>* specificnotifier, Lock &notifierlock);

    inline bool UnConnectFromNotifier(Lock &guard,
        BaseNotifier<ParentType, ChildType>* specificnotifier)
    {

        guard.unlock();
        GUARD_LOCK_OTHER_NAME(specificnotifier, guard2);
        guard.lock();
            
        return UnConnectFromNotifier(guard, specificnotifier, guard2);
    }

    //! \brief Notifies all the parents of this object about something
    //!
    //! This will call the BaseNotifier::OnNotified on all the child objects
    //! \param guard Lock for this object that needs to be able to be safely unlocked
    virtual void NotifyAll(Lock &guard);

    //! \brief Disconnects this from a previously connected notifier
    FORCE_INLINE bool UnConnectFromNotifier(
        BaseNotifier<ParentType, ChildType>* specificnotifier)
    {
        // The parent has to be locked before this object //
        GUARD_LOCK_OTHER_NAME(specificnotifier, guard2);
        GUARD_LOCK();
        return UnConnectFromNotifier(specificnotifier, guard, guard2);
    }

    //! \brief Actual implementation of this method
    bool IsConnectedTo(BaseNotifier<ParentType, ChildType>* check, Lock &guard);

    //! \brief Returns true when the specified object is already connected
    FORCE_INLINE bool IsConnectedTo(BaseNotifier<ParentType, ChildType>* check){
        GUARD_LOCK();
        return IsConnectedTo(check, guard);
    }

    //! \brief This searches the connected notifiers and calls the above function with it's pointer
    bool UnConnectFromNotifier(int id);

    //! \brief Connects this to a notifier object calling all the needed functions
    bool ConnectToNotifier(BaseNotifier<ParentType, ChildType>* owner);

    //! \brief Variant for already locked objects
    //! \param unlockable Lock that has this object locked and can be safely unlocked
    bool ConnectToNotifier(Lock &unlockable, BaseNotifier<ParentType, ChildType>* owner);

    //! Callback called by the parent, used to not to call the unhook again on the parent
    void _OnUnhookNotifier(Lock &locked, BaseNotifier<ParentType, ChildType>* parent,
        Lock &parentlock);

    //! Called by parent to hook, and doesn't call the parent's functions
    void _OnHookNotifier(Lock &locked, BaseNotifier<ParentType, ChildType>* parent,
        Lock &parentlock);

    //! \brief Gets the internal pointer to the actual object
    ChildType* GetActualPointerToNotifiableObject();

    //! \brief Called when our parent notifies us about something
    //! \note Both the parent and this object has been locked when this is called
    //! \warning Do not directly call this if you don't know what you are doing!
    virtual void OnNotified(Lock &ownlock, ParentType* parent, Lock &parentlock);


protected:

    // Callbacks for child classes to implement //
    virtual void _OnNotifierConnected(Lock &guard, ParentType* parentadded,
        Lock &parentlock);
    virtual void _OnNotifierDisconnected(Lock &guard, ParentType* parentremoved,
        Lock &parentlock);
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
    inline BaseNotifiableAll() : BaseNotifiable(this){
    }
    inline ~BaseNotifiableAll(){
    }
};
}

// The implementations are included here to make this compile //
#include "BaseNotifiableImpl.h"

#ifdef LEAK_INTO_GLOBAL
using Leviathan::BaseNotifiableAll;
#endif


#endif //BASENOTIFIABLE_H
