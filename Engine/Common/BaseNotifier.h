#ifndef BASENOTIFIER_H
#define BASENOTIFIER_H
#include "Define.h"
// ------------------------------------ //
#include "ThreadSafe.h"
#include <vector>

namespace Leviathan{

template<class ParentType, class ChildType>
	class BaseNotifier : public virtual ThreadSafe{
public:
    BaseNotifier(ParentType* ourptr);
    virtual ~BaseNotifier();

    //! Release function that unhooks all child objects
    inline void ReleaseChildHooks(){

        GUARD_LOCK();
        ReleaseChildHooks(guard);
    }

    void ReleaseChildHooks(Lock &guard);

    //! Connects this to a notifiable object for holding a reference to it
    FORCE_INLINE bool ConnectToNotifiable(
        BaseNotifiable<ParentType, ChildType>* child)
    {
        GUARD_LOCK();
        GUARD_LOCK_OTHER_NAME(child, guard2);
        return ConnectToNotifiable(guard, child, guard2);
    }

    //! \brief The actual implementation of ConnecToNotifiable
    bool ConnectToNotifiable(Lock &guard,
        BaseNotifiable<ParentType, ChildType>* child, Lock &childlock);


    //! \brief Notifies the children about something
    //!
    //! This will call the BaseNotifiable::OnNotified on all the child objects
    //! \param guard The lock for this object
    virtual void NotifyAll(Lock &guard);


    //! \brief Disconnects from previously connected notifiable
    bool UnConnectFromNotifiable(Lock &guard,
        BaseNotifiable<ParentType, ChildType>* unhookfrom);

    FORCE_INLINE bool UnConnectFromNotifiable(
        BaseNotifiable<ParentType, ChildType>* child)
    {
        GUARD_LOCK();
        return UnConnectFromNotifiable(guard, child);
    }

    //! \brief Searches the connected notifiable objects and calls the above function with it's pointer
    bool UnConnectFromNotifiable(int id);

    //! \brief Actual implementation of this method
    bool IsConnectedTo(BaseNotifiable<ParentType, ChildType>* check, Lock &guard);

    //! \brief Returns true when the specified object is already connected
    FORCE_INLINE bool IsConnectedTo(BaseNotifiable<ParentType, ChildType>* check){
        GUARD_LOCK();
        return IsConnectedTo(check, guard);
    }
		
    // Callback called by the child, and doesn't call the unhook again on the child
    void _OnUnhookNotifiable(Lock &guard,
        BaseNotifiable<ParentType, ChildType>* childtoremove, Lock &childlock);

    // Called by child to hook, and doesn't call the child's functions
    void _OnHookNotifiable(Lock &guard, BaseNotifiable<ParentType, ChildType>* child,
        Lock &childlock);

    //! \brief Gets the internal pointer to the actual object
    ParentType* GetActualPointerToNotifierObject();

    //! \brief Called when one of our children notifies us about something
    //! \note Both the child and this object has been locked when this is called
    //! \warning Do not directly call this if you don't know what you are doing!
    virtual void OnNotified(Lock &ownlock, ChildType* child, Lock &childlock);

protected:

    // Callbacks for child classes to implement //
    // This object should already be locked during this call //
    virtual void _OnNotifiableConnected(Lock &guard, ChildType* childadded,
        Lock &childlock);
    virtual void _OnNotifiableDisconnected(Lock &guard, ChildType* childtoremove,
        Lock &childlock);
    // ------------------------------------ //

    //! Stores a pointer to the object that is inherited from this
    ParentType* PointerToOurNotifier;

    //! Vector of other objects that this is connected to
    std::vector<BaseNotifiable<ParentType, ChildType>*> ConnectedChildren;
};

//! \brief Specialized class for accepting all parent/child objects
class BaseNotifierAll : public BaseNotifier<BaseNotifierAll, BaseNotifiableAll>{
public:
    inline BaseNotifierAll() : BaseNotifier(this){
    }
    inline ~BaseNotifierAll(){
    }
};
}

// The implementations are included here to make this compile //
#include "BaseNotifierImpl.h"

#ifdef LEAK_INTO_GLOBAL
using Leviathan::BaseNotifierAll;
#endif


#endif //BASENOTIFIER_H

