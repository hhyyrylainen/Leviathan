#pragma once
#include "Define.h"
// ------------------------------------ //
#include <memory>
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
        
        template<typename T>
        static T* TurnToPointer(T &obj){
            return &obj;
        }

        template<typename T>
        static T* TurnToPointer(T* obj){
            return obj;
        }
        
    public:


        template<typename ObjectClass>
        static auto Object(const ObjectClass* object){

            return Unique(TurnToPointer(object)->ObjectsLock);
        }

        template<typename ObjectClass>
        static auto Object(const ObjectClass &object){

            return Unique(TurnToPointer(object)->ObjectsLock);
        }

        template<typename ObjectClass>
        static auto Object(std::shared_ptr<ObjectClass> &object){

            return Unique(object->ObjectsLock);
        }

        template<typename ObjectClass>
        static auto Object(std::unique_ptr<ObjectClass> &object){

            return Unique(object->ObjectsLock);
        }

        template<class LockType>
        static auto Unique(LockType &lockref){

            return typename LockTypeResolver<LockType>::LType(lockref);
        }
    };
    
#if 0
    // These prevent copy elision
#define GUARD_LOCK() auto guard = std::move(Leviathan::Locker::Object(this));
    
#define GUARD_LOCK_OTHER(x) auto guard = std::move(Leviathan::Locker::Object(x));
#define GUARD_LOCK_NAME(y) auto y = std::move(Leviathan::Locker::Object(this));
#define GUARD_LOCK_OTHER_NAME(x,y) auto y = std::move(Leviathan::Locker::Object(x));
    
#define UNIQUE_LOCK_OBJECT_OTHER(x) auto lockit = std::move(Leviathan::Locker::Object(x));
#define UNIQUE_LOCK_THIS() auto lockit = std::move(Leviathan::Locker::Object(this));
#else
#define GUARD_LOCK() auto guard = (Leviathan::Locker::Object(this));
    
#define GUARD_LOCK_OTHER(x) auto guard = (Leviathan::Locker::Object(x));
#define GUARD_LOCK_NAME(y) auto y = (Leviathan::Locker::Object(this));
#define GUARD_LOCK_OTHER_NAME(x,y) auto y = (Leviathan::Locker::Object(x));
    
#define UNIQUE_LOCK_OBJECT_OTHER(x) auto lockit = (Leviathan::Locker::Object(x));
#define UNIQUE_LOCK_THIS() auto lockit = (Leviathan::Locker::Object(this));
#endif
    
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
            LEVIATHAN_ASSERT(lockit.owns_lock(), "lock not locked");
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

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Mutex;
using Leviathan::RecursiveMutex;
using Leviathan::Lock;
using Leviathan::RecursiveLock;
#endif // LEAK_INTO_GLOBAL

