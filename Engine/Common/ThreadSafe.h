#ifndef LEVIATHAN_THREADSAFE
#define LEVIATHAN_THREADSAFE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions.h"
#include "boost/thread/lockable_adapter.hpp"
#include "boost/thread/recursive_mutex.hpp"


namespace Leviathan{

    typedef boost::unique_lock<boost::mutex> BasicLockType;
    
	typedef boost::strict_lock<boost::recursive_mutex> ObjectLock;
	typedef boost::unique_lock<boost::recursive_mutex> UniqueObjectLock;
    typedef boost::unique_lock<boost::recursive_mutex> UObjectLock;

#define GUARD_LOCK_THIS_OBJECT()						ObjectLock guard(this->ObjectsLock);
#define GUARD_LOCK_THIS_OBJECT_CAST(BaseClass)			ObjectLock guard(static_cast<BaseClass*>(this)->ObjectsLock);
#define GUARD_LOCK_OTHER_OBJECT(x)						ObjectLock guard(x->ObjectsLock);
#define GUARD_LOCK_OTHER_OBJECT_NAME(x,y)				ObjectLock y(x->ObjectsLock);
#define GUARD_LOCK_OTHER_OBJECT_UNIQUE_PTR_NAME(x, y)	unique_ptr<ObjectLock> y(new ObjectLock(x->ObjectsLock));
#define UNIQUE_LOCK_OBJECT(x)							UniqueObjectLock lockit(x->ObjectsLock);
#define UNIQUE_LOCK_THIS_OBJECT()						UniqueObjectLock lockit(this->ObjectsLock);
#define GUARD_LOCK_BASIC(x)                             BasicLockType lock(x);

    // Individual lock objects //
    using Mutex = boost::mutex;
    using RecursiveMutex = boost::recursive_mutex;
    using Lock = boost::unique_lock<boost::mutex>;
    using RecursiveLock = boost::strict_lock<boost::recursive_mutex>;
    

	//! \brief Allows the inherited object to be locked
	class ThreadSafe{
	public:
		DLLEXPORT ThreadSafe();
		DLLEXPORT virtual ~ThreadSafe();

		FORCE_INLINE void VerifyLock(ObjectLock &guard) const THROWS{
            
			// Ensure that lock is for this //
			if(!guard.owns_lock(&this->ObjectsLock))
				throw InvalidAccess("wrong lock owner");
		}

        FORCE_INLINE void VerifyLock(UniqueObjectLock &lockit) const THROWS{
            
            // Make sure that the lock is locked //
			if(!lockit.owns_lock())
				throw InvalidAccess("lock not locked");
		}

		//! The main lock facility, mutable for working with const functions
		//! \note Even though this is not protected it should not be abused
		//! \protected
		mutable boost::recursive_mutex ObjectsLock;
	};

}
#endif
