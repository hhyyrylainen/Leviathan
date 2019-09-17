// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ObjectPool.h"
#include "Component.h"
#include "EntityCommon.h"
#include "Exceptions.h"
#include "GameWorld.h"
#include "StateHolder.h"


namespace Leviathan {

//! This holds many CachedComponentCollections which are a list of
//! components that are important for a system, and this avoids
//! looking them up on each tick
template<class T>
class CachedComponentCollectionHolder : public ObjectPool<T, ObjectID> {};

//! This is split from System to allow easily creation of systems that
//! have multiple CachedComponentCollection types
template<class UsedCachedComponentCollectionT>
class SystemCachedComponentCollectionStorage {
public:
    using HolderType = CachedComponentCollectionHolder<UsedCachedComponentCollectionT>;

    void Clear()
    {
        CachedComponents.Clear();
    }

    auto GetCachedComponentCollectionCount() const
    {
        return CachedComponents.GetObjectCount();
    }

    // TODO: do something about the amount of copy pasting done here


    //! \brief Helper function for creating nodes based on std::tuple
    //! \todo Also figure out if "CachedComponentCollections.Find(id) != nullptr" should
    //! be before _TupleHelperGetIfComponentExists
    template<class FirstType, class SecondType>
    static void TupleCachedComponentCollectionHelper(
        ObjectPool<std::tuple<FirstType&, SecondType&>, ObjectID>& CachedComponentCollections,
        const std::vector<std::tuple<FirstType*, ObjectID>>& firstdata,
        const std::vector<std::tuple<SecondType*, ObjectID>>& seconddata,
        const ComponentHolder<FirstType>& firstholder,
        const ComponentHolder<SecondType>& secondholder)
    {
        // First way around //
        for(auto iter = firstdata.begin(); iter != firstdata.end(); ++iter) {

            const auto id = std::get<1>(*iter);
            SecondType* other = _TupleHelperGetIfComponentExists(id, seconddata, secondholder);

            // Create node if required components where found and it doesn't exist already //
            if(!other || CachedComponentCollections.Find(id) != nullptr)
                continue;

            CachedComponentCollections.ConstructNew(id, *std::get<0>(*iter), *other);
        }

        // And the other way around //
        for(auto iter = seconddata.begin(); iter != seconddata.end(); ++iter) {

            const auto id = std::get<1>(*iter);
            FirstType* other = _TupleHelperGetIfComponentExists(id, firstdata, firstholder);

            // Create node if required components where found and it doesn't exist already //
            if(!other || CachedComponentCollections.Find(id) != nullptr)
                continue;

            CachedComponentCollections.ConstructNew(id, *other, *std::get<0>(*iter));
        }
    }

    //! \brief Tree part component TupleCachedComponentCollectionHelper
    template<class FirstType, class SecondType, class ThirdType>
    static void TupleCachedComponentCollectionHelper(
        ObjectPool<std::tuple<FirstType&, SecondType&, ThirdType&>, ObjectID>&
            CachedComponentCollections,
        const std::vector<std::tuple<FirstType*, ObjectID>>& firstdata,
        const std::vector<std::tuple<SecondType*, ObjectID>>& seconddata,
        const std::vector<std::tuple<ThirdType*, ObjectID>>& thirddata,
        const ComponentHolder<FirstType>& firstholder,
        const ComponentHolder<SecondType>& secondholder,
        const ComponentHolder<ThirdType>& thirdholder)
    {
        // First way around //
        for(auto iter = firstdata.begin(); iter != firstdata.end(); ++iter) {

            const auto id = std::get<1>(*iter);
            SecondType* other2 =
                _TupleHelperGetIfComponentExists(id, seconddata, secondholder);
            ThirdType* other3 = _TupleHelperGetIfComponentExists(id, thirddata, thirdholder);

            // Create node if required components where found and it doesn't exist already //
            if(!other2 || !other3 || CachedComponentCollections.Find(id) != nullptr)
                continue;

            CachedComponentCollections.ConstructNew(id, *std::get<0>(*iter), *other2, *other3);
        }

        // And the other way around //
        for(auto iter = seconddata.begin(); iter != seconddata.end(); ++iter) {

            const auto id = std::get<1>(*iter);
            FirstType* other1 = _TupleHelperGetIfComponentExists(id, firstdata, firstholder);
            ThirdType* other3 = _TupleHelperGetIfComponentExists(id, thirddata, thirdholder);

            // Create node if required components where found and it doesn't exist already //
            if(!other1 || !other3 || CachedComponentCollections.Find(id) != nullptr)
                continue;

            CachedComponentCollections.ConstructNew(id, *other1, *std::get<0>(*iter), *other3);
        }

        // Third way around //
        for(auto iter = thirddata.begin(); iter != thirddata.end(); ++iter) {

            const auto id = std::get<1>(*iter);
            FirstType* other1 = _TupleHelperGetIfComponentExists(id, firstdata, firstholder);
            SecondType* other2 =
                _TupleHelperGetIfComponentExists(id, seconddata, secondholder);

            // Create node if required components where found and it doesn't exist already //
            if(!other1 || !other2 || CachedComponentCollections.Find(id) != nullptr)
                continue;

            CachedComponentCollections.ConstructNew(id, *other1, *other2, *std::get<0>(*iter));
        }
    }

protected:
    // Helpers for TupleCachedComponentCollectionHelper //
    template<class T>
    static inline T* _TupleHelperGetIfComponentExists(ObjectID id,
        const std::vector<std::tuple<T*, ObjectID>>& addedlist,
        const ComponentHolder<T>& holder)
    {
        // First search added //
        for(auto iter = addedlist.begin(); iter != addedlist.end(); ++iter) {

            if(std::get<1>(*iter) == id) {

                return std::get<0>(*iter);
            }
        }

        // This full search returns a nullptr if not found //
        return holder.Find(id);
    }

public:
    HolderType CachedComponents;
};

//! \brief Base for all entity component related systems
//!
//! For ones that use nodes. Not for ones that directly use a single component type
//! \todo It would bemore efficient to directly create nodes as entities are created instead
//! of running CreateCachedComponentCollections (implemented in subclasses of this)
template<class UsedCachedComponentCollection>
class System : public SystemCachedComponentCollectionStorage<UsedCachedComponentCollection> {
public:
    /* Template for node run method, copy-paste and fill in the parameters

        void Run(GameWorld &world){

        auto& index = CachedComponents.GetIndex();
        for(auto iter = index.begin(); iter != index.end(); ++iter){

            this->ProcessCachedComponents(*iter->second, iter->first, );
        }
    */
};

//! \brief Base class for systems that use a single component directly
//! \note This basically has nothing so this doesn't need to be used
template<class UsedComponent>
class SingleSystem {
public:
    // Example run method
    // void Run(GameWorld &world, std::unordered_map<ObjectID, UsedComponent*> &index);
};

//! \brief Base class for all systems that create states from changed components
template<class UsedComponent, class ComponentState>
class StateCreationSystem {
public:
    void Run(GameWorld& world, std::unordered_map<ObjectID, UsedComponent*>& index,
        StateHolder<ComponentState>& heldstates, int worldtick)
    {
        // TODO: find a better way (see the comment a few lines down why this is here)
        if(!world.GetNetworkSettings().DoInterpolation)
            return;

        const bool authoritative = world.GetNetworkSettings().IsAuthoritative;

        for(auto iter = index.begin(); iter != index.end(); ++iter) {
            auto& component = *iter->second;

            if(!component.Marked)
                continue;

            // And only for locally controlled entities
            if(!authoritative && !world.IsUnderOurLocalControl(iter->first))
                continue;

            // Ignore creating states on the server when using local control as that causes
            // issues Actually this whole system is disabled when interpolating isn't needed

            // Needs a new state //
            if(heldstates.CreateStateIfChanged(iter->first, component, worldtick)) {

                component.StateMarked = true;
            }

            component.Marked = false;
        }
    }
};


} // namespace Leviathan
