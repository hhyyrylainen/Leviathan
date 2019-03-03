// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Include.h"
// ------------------------------------ //

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

//! \brief A tiny wrapper around boost pool
template<class ElementType>
class BasicPool {
public:
    BasicPool()
#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        :
        Elements(sizeof(ElementType), 16)
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
    {}

    ~BasicPool() {}

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

    //! \brief Constructs a new component of the held type for entity
    //! \exception Exception when component has not been created
    template<typename... Args>
    ElementType* ConstructNew(Args&&... args)
    {
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

        return created;
    }

    //! \brief Destroys a created element
    void Destroy(ElementType* element)
    {

        element->~ElementType();
        Elements.free(element);
    }
#else
    template<typename... Args>
    ElementType* ConstructNew(Args&&... args)
    {
        return new ElementType(std::forward<Args>(args)...);
    }

    void Destroy(ElementType* element)
    {
        delete element;
    }

#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

protected:
#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
    //! Pool for objects
    boost::pool<> Elements;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
};

//! \brief Creates objects in a shared memory region
//! \todo Should this class use the ordered_malloc family of methods to allow better use
//! of memory blocks (but is slightly slower for a large number of allocations)
template<class ElementType, typename KeyType, bool AutoCleanupObjects = true>
class ObjectPool {
public:
#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
    //! \todo Figure out the optimal value for the Elements constructor (initial size)
    ObjectPool() : Elements(sizeof(ElementType), 16) {}
#else
    ObjectPool() {}
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

    ~ObjectPool()
    {
        if(!AutoCleanupObjects)
            return;

        Clear();
    }

    //! \brief Constructs a new component of the held type for entity
    //! \exception Exception when component has not been created
    template<typename... Args>
    ElementType* ConstructNew(KeyType forentity, Args&&... args)
    {
        if(Find(forentity))
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
        return created;
    }

    //! \brief Calls Release on an object and then removes it from the pool
    template<typename... Args>
    void Release(KeyType entity, Args&&... args)
    {
        auto* object = Find(entity);

        if(!object)
            throw NotFound("entity not in pool");

        object->Release(std::forward<Args>(args)...);

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        object->~ElementType();
        Elements.free(object);
#else
        delete object;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

        RemoveFromIndex(entity);
        RemoveFromAdded(entity);
    }

    //! \return The found component or NULL
    ElementType* Find(KeyType id) const
    {
        auto iter = Index.find(id);

        if(iter == Index.end())
            return nullptr;

        return iter->second;
    }

    //! \brief Destroys a component based on id
    void Destroy(KeyType id)
    {
        auto object = Find(id);

        if(!object)
            throw InvalidArgument("ID is not in index");

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        object->~ElementType();
        Elements.free(object);
#else
        delete object;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

        RemoveFromIndex(id);
        RemoveFromAdded(id);
    }

    //! \brief Destroys without releasing elements based on ids in vector
    template<typename Any>
    void RemoveBasedOnKeyTupleList(const std::vector<std::tuple<Any, KeyType>>& values)
    {
        for(auto iter = values.begin(); iter != values.end(); ++iter) {

            auto todelete = Index.find(std::get<1>(*iter));

            if(todelete == Index.end())
                continue;

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
            todelete->second->~ElementType();
            Elements.free(todelete->second);
#else
            delete todelete->second;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

            Index.erase(todelete);
        }
    }


    //! \brief Calls an function on all the objects in the pool
    //! \note The order of the objects is not guaranteed and can change between runs
    //! \param function The function that is called with all of the components of this type
    //! the first parameter is the component, the second is the id of the entity owning the
    //! component, the return value specifies
    //! if the component should be destroyed (true being yes and false being no)
    void Call(std::function<bool(ElementType&, KeyType)> function)
    {
        for(auto iter = Index.begin(); iter != Index.end();) {

            if(function(*iter->second, iter->first)) {

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
        for(auto iter = Index.begin(); iter != Index.end(); ++iter) {

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
            iter->second->~ElementType();
            Elements.free(iter->second);
#else
            delete iter->second;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        }

        Index.clear();
    }

    auto GetObjectCount() const
    {
        return Index.size();
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
    bool RemoveFromIndex(KeyType id)
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

protected:
    //! Used for looking up element belonging to id
    std::unordered_map<KeyType, ElementType*> Index;

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
    //! Pool for objects
    boost::pool<> Elements;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
};

// ------------------------------------ //
//! \brief Creates objects in a shared memory region
//!
//! Tracks the creation and destruction of elements as needed by components in a GameWorld
//! \todo Created and destroyed vectors could use swap with last and remove the last to
//! remove more effectively
template<class ElementType, typename KeyType, bool AutoCleanupObjects = true>
class ObjectPoolTracked {
public:
    ObjectPoolTracked()
#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        :
        Elements(sizeof(ElementType), 16)
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
    {}

    ~ObjectPoolTracked()
    {
        if(!AutoCleanupObjects)
            return;

        Clear();
    }

    //! \brief Constructs a new component of the held type for entity
    //! \exception Exception when component has not been created
    template<typename... Args>
    ElementType* ConstructNew(KeyType forentity, Args&&... args)
    {
        if(Find(forentity))
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

    //! \brief Returns true if there are objects in Removed
    bool HasElementsInRemoved() const
    {
        return !Removed.empty();
    }

    //! \brief Returns true if there are objects in Added
    bool HasElementsInAdded() const
    {
        return !Added.empty();
    }

    //! \brief Returns true if there are objects in Queued
    bool HasElementsInQueued() const
    {
        return !Queued.empty();
    }

    //! \brief Calls Release with the specified arguments on elements that are queued
    //! for destruction
    template<typename... Args>
    void ReleaseQueued(Args&&... args)
    {
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
            RemoveFromIndex(id);
        }

        Queued.clear();
    }

    //! \brief Calls Release on an object and then removes it from the pool
    template<typename... Args>
    void Release(KeyType id, bool addtoremoved, Args&&... args)
    {
        auto* object = Find(id);

        if(!object)
            throw NotFound("id not in pool");

        _ReleaseCommon(object, id, addtoremoved, std::forward<Args>(args)...);
    }

    //! \brief Calls Release on an object if it is in this pool and
    //! then removes it from the pool
    template<typename... Args>
    bool ReleaseIfExists(KeyType id, bool addtoremoved, Args&&... args)
    {
        auto* object = Find(id);

        if(!object)
            return false;

        _ReleaseCommon(object, id, addtoremoved, std::forward<Args>(args)...);
        return true;
    }

    //! \brief Removes elements that are queued for destruction
    //! without calling release
    void ClearQueued()
    {
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
            RemoveFromIndex(id);
        }

        Queued.clear();
    }

    //! \brief Returns a reference to the vector of removed elements
    const auto& GetRemoved() const
    {
        return Removed;
    }

    //! \brief Returns a reference to the vector of added elements
    auto& GetAdded()
    {
        return Added;
    }

    //! \brief Clears the added list
    void ClearAdded()
    {
        Added.clear();
    }

    //! \brief Clears the removed list
    void ClearRemoved()
    {
        Removed.clear();
    }

    //! \brief Destroys without releasing elements based on ids in vector
    //! \param addtoremoved If true will add the elements to the Removed index
    //! for (possibly) using them to remove attached resources
    template<typename Any>
    void RemoveBasedOnKeyTupleList(
        const std::vector<std::tuple<Any, KeyType>>& values, bool addtoremoved = false)
    {
        for(auto iter = values.begin(); iter != values.end(); ++iter) {

            auto todelete = Index.find(std::get<1>(*iter));

            if(todelete == Index.end())
                continue;

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
            todelete->second->~ElementType();
            Elements.free(todelete->second);
#else
            delete todelete->second;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

            if(addtoremoved)
                Removed.push_back(std::make_tuple(todelete->second, todelete->first));

            RemoveFromAdded(todelete->first);

            Index.erase(todelete);
        }
    }

    //! \brief Calls release on all objects and clears everything
    template<typename... Args>
    void ReleaseAllAndClear(Args&&... args)
    {
        for(auto iter = Index.begin(); iter != Index.end(); ++iter) {

            auto object = iter->second;

            object->Release(std::forward<Args>(args)...);

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
            object->~ElementType();
            Elements.free(object);
#else
            delete object;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        }

        // Skip double free
        Index.clear();

        // And then clear the rest of things
        Clear();
    }

    //! \brief Removes a specific id from the added list
    //!
    //! Used to remove entries that have been deleted before clearing the added ones
    //! \todo Check if this gives better performance than noticing missing elements
    //! during node construction
    void RemoveFromAdded(KeyType id)
    {
        for(auto iter = Added.begin(); iter != Added.end(); ++iter) {

            if(std::get<1>(*iter) == id) {

                Added.erase(iter);
                return;
            }
        }
    }

    //! \return The found component or NULL
    ElementType* Find(KeyType id) const
    {
        auto iter = Index.find(id);

        if(iter == Index.end())
            return nullptr;

        return iter->second;
    }

    //! \brief Destroys a component based on id
    void Destroy(KeyType id, bool addtoremoved = true)
    {
        auto object = Find(id);

        if(!object)
            throw InvalidArgument("ID is not in index");

        _DestroyCommon(object, id, addtoremoved);
    }

    //! \brief Destroys a component based on id if exists
    bool DestroyIfExists(KeyType id, bool addtoremoved = true)
    {
        auto object = Find(id);

        if(!object)
            return false;

        _DestroyCommon(object, id, addtoremoved);
        return true;
    }

    //! \brief Queues destruction of an element
    //! \exception InvalidArgument when key is not found (is already deleted)
    //! \note This has to be used for objects that require calling Release
    void QueueDestroy(KeyType id)
    {
        auto end = Index.end();
        for(auto iter = Index.begin(); iter != end; ++iter) {

            if(iter->first == id) {

                Queued.push_back(std::make_tuple(iter->second, id));

                RemoveFromAdded(id);

                return;
            }
        }

        throw InvalidArgument("ID is not in index");
    }

    //! \brief Calls an function on all the objects in the pool
    //! \note The order of the objects is not guaranteed and can change between runs
    //! \param function The function that is called with all of the components of this type
    //! the first parameter is the component, the second is the id of the entity owning the
    //! component, the return value specifies
    //! if the component should be destroyed (true being yes and false being no)
    void Call(std::function<bool(ElementType&, KeyType)> function)
    {
        for(auto iter = Index.begin(); iter != Index.end();) {

            if(function(*iter->second, iter->first)) {

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

    auto GetObjectCount() const
    {
        return Index.size();
    }

    //! \brief Returns a direct access to Index
    //! \note Do not change the returned index it is intended only for looping.
    //! Okay, you may change it but you have to be extremely careful
    inline std::unordered_map<KeyType, ElementType*>& GetIndex()
    {
        return Index;
    }

    inline auto GetIndexSize() const
    {
        return Index.size();
    }

    //! \brief Returns a created object by index
    //!
    //! Helper for exposing object pools to scripts
    //! \note This is not optimal. It seems that with the map approach it's not possible to
    //! have good random iterator access
    inline ElementType* GetAtIndex(size_t index)
    {
        if(index >= Index.size())
            throw InvalidArgument("index out of range");

        auto iter = Index.begin();

        for(size_t i = 0; i < index; ++i)
            ++iter;
        return iter->second;
    }

protected:
    //! \brief Removes an component from the index but doesn't destruct it
    //! \note The component will only be deallocated once this object is destructed
    bool RemoveFromIndex(KeyType id)
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

    template<typename... Args>
    void _ReleaseCommon(ElementType* object, KeyType id, bool addtoremoved, Args&&... args)
    {
        object->Release(std::forward<Args>(args)...);

#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        object->~ElementType();
        Elements.free(object);
#else
        delete object;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS



        if(addtoremoved)
            Removed.push_back(std::make_tuple(object, id));

        RemoveFromIndex(id);
        RemoveFromAdded(id);
    }

    void _DestroyCommon(ElementType* object, KeyType id, bool addtoremoved)
    {
#ifdef LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
        object->~ElementType();
        Elements.free(object);
#else
        delete object;
#endif // LEVIATHAN_USE_ACTUAL_OBJECT_POOLS

        if(addtoremoved)
            Removed.push_back(std::make_tuple(object, id));

        RemoveFromIndex(id);
        RemoveFromAdded(id);
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
