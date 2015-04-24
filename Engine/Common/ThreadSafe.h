#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions.h"
#include <mutex>


namespace Leviathan{

    // Individual lock objects //
    using Mutex = std::mutex;
    using RecursiveMutex = std::recursive_mutex;
    using Lock = std::unique_lock<std::mutex>;
    using RecursiveLock = std::lock_guard<std::recursive_mutex>;

    template<class LockType>
    struct LockTypeResolver{

        using LType = void;
    };

    template<> struct LockTypeResolver<Mutex>{

        using LType = Lock;
    };

    template<> struct LockTypeResolver<RecursiveMutex>{

        using LType = RecursiveLock;
    };

    class Locker{
    public:
        
        template<class LockType>
        static auto Unique(LockType &lockref){

            return typename LockTypeResolver<LockType>::LType(lockref);
        }

    };
        

#define GUARD_LOCK() auto guard = std::move(Locker::Unique(this->ObjectsLock));
#define GUARD_LOCK_CAST(BaseClass) Lock guard(static_cast<BaseClass*>(this)->ObjectsLock);
#define GUARD_LOCK_OTHER(x) Lock guard(x->ObjectsLock);
#define GUARD_LOCK_NAME(y) Lock y(this->ObjectsLock);
#define GUARD_LOCK_NAME_OTHER(x,y) Lock y(x->ObjectsLock);
#define GUARD_LOCK_UNIQUE_PTR(y) unique_ptr<Lock> y(new Lock(this->ObjectsLock));
#define GUARD_LOCK_UNIQUE_PTR_OTHER(x, y)	unique_ptr<Lock> y(new Lock(x->ObjectsLock));
    
#define UNIQUE_LOCK_OBJECT_OTHER(x) Lock lockit(x->ObjectsLock);
#define UNIQUE_LOCK_OBJECT() Lock lockit(this->ObjectsLock);
    

	//! \brief Allows the inherited object to be locked
    //! \note Not allowed to be used as a pointer type
    template<class MutexType>
	class ThreadSafeGeneric{
	public:
		DLLEXPORT ThreadSafeGeneric(){}
		DLLEXPORT ~ThreadSafeGeneric(){}

		FORCE_INLINE void VerifyLock(RecursiveLock &guard) const{
            // Apparently there is no way to verify this...
			// if(!guard.owns_lock(&this->ObjectsLock))
			// 	throw InvalidAccess("wrong lock owner");
		}

        FORCE_INLINE void VerifyLock(Lock &lockit) const{
            
            // Make sure that the lock is locked //
			if(!lockit.owns_lock())
				throw InvalidAccess("lock not locked");
		}

		//! The main lock facility, mutable for working with const functions
		//! \note Even though this is not protected it should not be abused
		//! \protected
		mutable MutexType ObjectsLock;
	};

    using ThreadSafe = ThreadSafeGeneric<Mutex>;
    using ThreadSafeRecursive = ThreadSafeGeneric<RecursiveMutex>;
    

}

