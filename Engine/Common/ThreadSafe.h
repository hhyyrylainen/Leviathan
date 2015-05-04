#pragma once
// ------------------------------------ //
#include "Include.h"
#include "../Exceptions.h"
#include <mutex>
#include <memory>

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
    
#define GUARD_LOCK_CAST(BaseClass) auto guard = std::move(Locker::Unique(static_cast<BaseClass*>(\
                this)->ObjectsLock));
    
#define GUARD_LOCK_OTHER(x) auto guard = std::move(Locker::Unique(x->ObjectsLock));
#define GUARD_LOCK_NAME(y) auto y = std::move(Locker::Unique(this->ObjectsLock));
#define GUARD_LOCK_OTHER_NAME(x,y) auto y = std::move(Locker::Unique(x->ObjectsLock));
    
#define UNIQUE_LOCK_OBJECT_OTHER(x) auto lockit = std::move(Locker::Unique(x->ObjectsLock));
#define UNIQUE_LOCK_THIS() auto lockit = std::move(Locker::Unique(this->ObjectsLock));
    

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

    //! \brief Simple lockable objects, no recursive locking
    using ThreadSafe = ThreadSafeGeneric<Mutex>;
    //! \brief Object supports recursive locking
    //!
    //! Less efficient than ThreadSafe
    using ThreadSafeRecursive = ThreadSafeGeneric<RecursiveMutex>;
    

}

using Leviathan::Mutex;
using Leviathan::RecursiveMutex;
using Leviathan::Lock;
using Leviathan::RecursiveLock;


