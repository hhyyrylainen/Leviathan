// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "ThreadSafe.h"

#include "Exceptions.h"

#include <functional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
//#include "boost/pool/object_pool.hpp"
#include "boost/pool/pool.hpp"
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS


namespace Leviathan {

//! \brief Thread safe version of ObjectPool
template<class ElementType, typename KeyType, bool AutoCleanupObjects = true>
class ThreadSafeObjectPool : public ThreadSafe {
public:
    ThreadSafeObjectPool()
#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        :
        Elements(sizeof(ElementType), 100, 200)
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
    {}

    ~ThreadSafeObjectPool()
    {
        if(!AutoCleanupObjects)
            return;

        Clear();
    }

    //! \brief Constructs a new component of the held type for entity
    //! \exception Exception when component has not been created
    template<typename... Args>
    ElementType* ConstructNew(Lock& guard, KeyType forentity, Args&&... args)
    {
        if(Find(guard, forentity))
            throw Exception("Entity with ID already has object in pool of this type");

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        // Get memory to hold the object //
        void* memoryForObject = Elements.malloc();

        if(!memoryForObject)
            throw Exception("Out of memory for element");

        // Construct the object //
        ElementType* created;
        try {
            created = new(memoryForObject) ElementType(std::forward<Args>(args)...);
        } catch(...) {

            Elements.free(memoryForObject);
            throw;
        }
#else
        // Construct the object //
        ElementType* created = new ElementType(std::forward<Args>(args)...);
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

        // Add to index for finding later //
        Index.insert(std::make_pair(forentity, created));

        Added.push_back(std::make_tuple(created, forentity));

        return created;
    }

    template<typename... Args>
    inline ElementType* ConstructNew(KeyType forentity, Args&&... args)
    {
        GUARD_LOCK();
        return ConstructNew(guard, forentity, args...);
    }

    //! \brief Returns true if there are objects in Removed
    bool HasElementsInRemoved() const
    {
        GUARD_LOCK();
        return !Removed.empty();
    }

    //! \brief Returns true if there are objects in Added
    bool HasElementsInAdded() const
    {
        GUARD_LOCK();
        return !Added.empty();
    }

    //! \brief Returns true if there are objects in Queued
    bool HasElementsInQueued() const
    {
        GUARD_LOCK();
        return !Queued.empty();
    }

    //! \brief Calls Release with the specified arguments on elements that are queued
    //! for destruction
    template<typename... Args>
    void ReleaseQueued(Args&&... args)
    {
        GUARD_LOCK();

        for(auto iter = Queued.begin(); iter != Queued.end(); ++iter) {

            auto object = std::get<0>(*iter);
            const auto id = std::get<1>(*iter);

            object->Release(std::forward<Args>(args)...);

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
            object->~ElementType();
            Elements.free(object);
#else
            delete object;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

            Removed.push_back(std::make_tuple(object, id));
            RemoveFromIndex(guard, id);
        }

        Queued.clear();
    }

    //! \brief Calls Release on an object and then removes it from the pool
    template<typename... Args>
    void Release(KeyType entity, Args&&... args)
    {
        GUARD_LOCK();

        auto* object = Find(guard, entity);

        if(!object)
            throw NotFound("entity not in pool");

        object->Release(std::forward<Args>(args)...);

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        object->~ElementType();
        Elements.free(object);
#else
        delete object;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

        RemoveFromIndex(guard, entity);
        RemoveFromAdded(guard, entity);
    }

    //! \brief Removes elements that are queued for destruction
    //! without calling release
    void ClearQueued()
    {
        GUARD_LOCK();

        for(auto iter = Queued.begin(); iter != Queued.end(); ++iter) {

            auto object = std::get<0>(*iter);
            const auto id = std::get<1>(*iter);

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
            object->~ElementType();
            Elements.free(object);
#else
            delete object;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

            Removed.push_back(std::make_tuple(object, id));
            RemoveFromIndex(guard, id);
        }

        Queued.clear();
    }

    //! \brief Returns a reference to the vector of removed elements
    //! \note This object needs to be locked and kept locked while using the result
    //! of this call
    const auto& GetRemoved(Lock& locked) const
    {
        return Removed;
    }



    //! \brief Returns a reference to the vector of added elements
    //! \note This object needs to be locked and kept locked while using the result
    //! of this call
    auto& GetAdded(Lock& locked)
    {
        return Added;
    }

    //! \brief Clears the added list
    inline void ClearAdded()
    {
        GUARD_LOCK();
        ClearAdded(guard);
    }

    void ClearAdded(Lock& guard)
    {
        Added.clear();
    }

    //! \brief Clears the removed list
    void ClearRemoved()
    {
        GUARD_LOCK();
        Removed.clear();
    }

    //! \brief Destroys without releasing elements based on ids in vector
    //! \param addtoremoved If true will add the elements to the Removed index
    //! for (possibly) using them to remove attached resources
    template<typename Any>
    void RemoveBasedOnKeyTupleList(
        const std::vector<std::tuple<Any, KeyType>>& values, bool addtoremoved = false)
    {
        GUARD_LOCK();

        for(auto iter = values.begin(); iter != values.end(); ++iter) {

            auto todelete = Index.find(std::get<1>(*iter));

            if(todelete == Index.end())
                continue;

            if(addtoremoved) {

                Removed.push_back(std::make_tuple(todelete->second, todelete->first));
            }

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
            todelete->second->~ElementType();
            Elements.free(todelete->second);
#else
            delete todelete->second;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

            RemoveFromAdded(guard, todelete->first);

            Index.erase(todelete);
        }
    }

    //! \brief Removes a specific id from the added list
    //!
    //! Used to remove entries that have been deleted before clearing the added ones
    //! \todo Check if this gives better performance than noticing missing elements
    //! during node construction
    void RemoveFromAdded(Lock& guard, KeyType id)
    {
        for(auto iter = Added.begin(); iter != Added.end(); ++iter) {

            if(std::get<1>(*iter) == id) {

                Added.erase(iter);
                return;
            }
        }
    }

    //! \return The found component or NULL
    ElementType* Find(Lock& guard, KeyType id) const
    {
        auto iter = Index.find(id);

        if(iter == Index.end())
            return nullptr;

        return iter->second;
    }

    inline ElementType* Find(KeyType id) const
    {
        GUARD_LOCK();
        return Find(guard, id);
    }

    //! \brief Destroys a component based on id
    void Destroy(Lock& guard, KeyType id, bool addtoremoved = true)
    {
        auto object = Find(guard, id);

        if(!object)
            throw InvalidArgument("ID is not in index");

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        object->~ElementType();
        Elements.free(object);
#else
        delete object;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

        if(addtoremoved)
            Removed.push_back(std::make_tuple(object, id));

        RemoveFromIndex(guard, id);
        RemoveFromAdded(guard, id);
    }

    inline void Destroy(KeyType id, bool addtoremoved = true)
    {
        GUARD_LOCK();
        Destroy(guard, id, addtoremoved);
    }

    //! \brief Queues destruction of an element
    //! \exception InvalidArgument when key is not found (is already deleted)
    //! \note This has to be used for objects that require calling Release
    void QueueDestroy(Lock& guard, KeyType id)
    {
        auto end = Index.end();
        for(auto iter = Index.begin(); iter != end; ++iter) {

            if(iter->first == id) {

                Queued.push_back(std::make_tuple(iter->second, id));

                RemoveFromAdded(guard, id);

                return;
            }
        }

        throw InvalidArgument("ID is not in index");
    }

    inline void QueueDestroy(KeyType id)
    {
        GUARD_LOCK();
        QueueDestroy(guard, id);
    }

    //! \brief Calls an function on all the objects in the pool
    //! \note The order of the objects is not guaranteed and can change between runs
    //! \param function The function that is called with all of the components of this type
    //! the first parameter is the component, the second is the id of the entity owning the
    //! component and the last one is a lock to this ComponentHolder,
    //! the return value specifies
    //! if the component should be destroyed (true being yes and false being no)
    void Call(std::function<bool(ElementType&, KeyType, Lock&)> function)
    {
        GUARD_LOCK();

        for(auto iter = Index.begin(); iter != Index.end();) {

            if(function(*iter->second, iter->first, guard)) {

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
                iter->second->~ElementType();
                Elements.free(iter->second);
#else
                delete iter->second;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

                iter = Index.erase(iter);

            } else {

                ++iter;
            }
        }
    }

    //! \brief Clears the index and replaces the pool with a new one
    //! \warning All objects after this call are invalid
    void Clear()
    {
        GUARD_LOCK();

        for(auto iter = Index.begin(); iter != Index.end(); ++iter) {

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
            iter->second->~ElementType();
            Elements.free(iter->second);
#else
            delete iter->second;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        }

        Index.clear();
        Removed.clear();
        Queued.clear();
        Added.clear();
    }

    //! \brief Returns a direct access to Index
    //! \note Do not change the returned index it is intended only for looping.
    //! Okay, you may change it but you have to be extremely careful
    inline std::unordered_map<KeyType, ElementType*>& GetIndex()
    {
        return Index;
    }

protected:
    //! \brief Removes an component from the index but doesn't destruct it
    //! \note The component will only be deallocated once this object is destructed
    bool RemoveFromIndex(Lock& guard, KeyType id)
    {
        auto end = Index.end();
        for(auto iter = Index.begin(); iter != end; ++iter) {

            if(iter->first == id) {

                Index.erase(iter);
                return true;
            }
        }

        return false;
    }

    inline bool RemoveFromIndex(KeyType id)
    {
        GUARD_LOCK();
        return RemoveFromIndex(id);
    }

protected:
    //! Used for looking up element belonging to id
    std::unordered_map<KeyType, ElementType*> Index;

    //! Used for detecting deleted elements later
    //! Can be used to unlink resources that have pointers to
    //! elements
    std::vector<std::tuple<ElementType*, KeyType>> Removed;

    //! Used for marking elements to be deleted later at a suitable
    //! time
    //! GameWorld uses this to control when components are deleted
    std::vector<std::tuple<ElementType*, KeyType>> Queued;

    //! Used for detecting created elements
    std::vector<std::tuple<ElementType*, KeyType>> Added;

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
    //! Pool for objects
    boost::pool<> Elements;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
};

} // namespace Leviathan
