// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Logger.h"

#include <memory>
#include <mutex>
#include <type_traits>

namespace Leviathan {


// Individual lock objects //
using Mutex = std::mutex;
using RecursiveMutex = std::recursive_mutex;
using Lock = std::unique_lock<std::mutex>;
using RecursiveLock = std::lock_guard<std::recursive_mutex>;

template<class LockType>
struct LockTypeResolver;
// {
//
//    using LType = void;
//};

template<>
struct LockTypeResolver<Mutex> {

    using LType = Lock;
};

template<>
struct LockTypeResolver<RecursiveMutex> {

    using LType = RecursiveLock;
};


class Locker {

    template<typename T>
    static T* TurnToPointer(const std::shared_ptr<T>& obj)
    {
        return obj.get();
    }

    template<typename T>
    static T* TurnToPointer(const std::unique_ptr<T>& obj)
    {
        return obj.get();
    }

    template<typename T>
    static T* TurnToPointer(T& obj)
    {
        return &obj;
    }

    template<typename T>
    static T* TurnToPointer(T* obj)
    {
        return obj;
    }

public:
    template<typename ObjectClass>
    static auto& AcessLock(const ObjectClass* object)
    {
        return TurnToPointer(object)->ObjectsLock;
    }

    template<typename ObjectClass>
    static auto& AcessLock(const ObjectClass& object)
    {
        return TurnToPointer(object)->ObjectsLock;
    }

    template<typename ObjectClass>
    static auto Object(const ObjectClass* object)
    {
        return Unique(TurnToPointer(object)->ObjectsLock);
    }

    template<typename ObjectClass>
    static auto Object(const ObjectClass& object)
    {
        return Unique(TurnToPointer(object)->ObjectsLock);
    }

    template<typename ObjectClass>
    static auto Object(std::shared_ptr<ObjectClass>& object)
    {
        return Unique(object->ObjectsLock);
    }

    template<typename ObjectClass>
    static auto Object(std::unique_ptr<ObjectClass>& object)
    {
        return Unique(object->ObjectsLock);
    }

    template<class LockType>
    static auto Unique(LockType& lockref)
    {
        return typename LockTypeResolver<LockType>::LType(lockref);
    }
};

#define GUARD_LOCK()                                                                  \
    typename Leviathan::LockTypeResolver<                                             \
        std::remove_reference_t<decltype(Leviathan::Locker::AcessLock(this))>>::LType \
        guard(Leviathan::Locker::AcessLock(this));

#define GUARD_LOCK_NAME(y)                                                 \
    typename Leviathan::LockTypeResolver<std::remove_reference_t<decltype( \
        Leviathan::Locker::AcessLock(this))>>::LType y(Leviathan::Locker::AcessLock(this));

#define GUARD_LOCK_OTHER(x)                                                \
    typename Leviathan::LockTypeResolver<std::remove_reference_t<decltype( \
        Leviathan::Locker::AcessLock(x))>>::LType guard(Leviathan::Locker::AcessLock(x));

#define GUARD_LOCK_OTHER(x)                                                \
    typename Leviathan::LockTypeResolver<std::remove_reference_t<decltype( \
        Leviathan::Locker::AcessLock(x))>>::LType guard(Leviathan::Locker::AcessLock(x));
#define GUARD_LOCK_OTHER_NAME(x, y)                                        \
    typename Leviathan::LockTypeResolver<std::remove_reference_t<decltype( \
        Leviathan::Locker::AcessLock(x))>>::LType y(Leviathan::Locker::AcessLock(x));

//! Asserts if lock isn't locked / doesn't own mutex
#define REQUIRE_LOCKED(x) LEVIATHAN_ASSERT(x.owns_lock(), "Mutex doesn't own lock");

//! \brief Allows the inherited object to be locked
//! \note Not allowed to be used as a pointer type
template<class MutexType>
class ThreadSafeGeneric {
public:
    using LockT = typename LockTypeResolver<MutexType>::LType;

    DLLEXPORT ThreadSafeGeneric() {}
    DLLEXPORT ~ThreadSafeGeneric() {}

    FORCE_INLINE void VerifyLock(RecursiveLock& guard) const
    {
        // Apparently there is no way to verify this...
        // if(!guard.owns_lock(&this->ObjectsLock))
        // 	throw InvalidAccess("wrong lock owner");
    }

    FORCE_INLINE void VerifyLock(Lock& lockit) const
    {
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

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Lock;
using Leviathan::Mutex;
using Leviathan::RecursiveLock;
using Leviathan::RecursiveMutex;

using Leviathan::ThreadSafe;
#endif // LEAK_INTO_GLOBAL
