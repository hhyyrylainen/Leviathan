// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"

#include "Component.h"
#include "EntityCommon.h"

#include "Common/ObjectPool.h"
#include "Exceptions.h"


namespace Leviathan{

template<class NodeType>
    class NodeHolder : public ObjectPool<NodeType, ObjectID>{
public:

};
    
//! \brief Base for all entity component related systems
//!
//! For ones that use nodes. Not for ones that directly use a single component type
template<class UsedNode>
class System {
public:

    using HolderType = NodeHolder<UsedNode>;

    //! \brief Runs this system on its nodes
    //!
    //! \note The nodes need to be updated before calling this, otherwise some entities
    //! might not be picked up
    virtual void Run(GameWorld &world) = 0;

protected:

    //! \brief Helper for Run
    //!
    //! Goes through all nodes and calls func on them
    template <class T, void(T::*F)(UsedNode &node, ObjectID nodesobject)>
    void RunAllNodes(T &instance) {
        auto& index = Nodes.GetIndex();
        for (auto iter = index.begin(); iter != index.end(); ++iter) {

            (instance.*F)(*iter->second, iter->first);
        }
    }


    //! \brief Helper function for creating nodes based on std::tuple 
    template<class FirstType, class SecondType>
    void TupleNodeHelper(
        ObjectPool<std::tuple<FirstType&, SecondType&>, ObjectID> &Nodes,
        const std::vector<std::tuple<FirstType*, ObjectID>> &firstdata,
        const std::vector<std::tuple<SecondType*, ObjectID>> &seconddata,
        const ComponentHolder<FirstType> &firstholder, Lock &firstlock,
        const ComponentHolder<SecondType> &secondholder, Lock &secondlock) 
    {
        GUARD_LOCK_OTHER(Nodes);

        for (auto iter = firstdata.begin(); iter != firstdata.end(); ++iter) {

            SecondType* other = nullptr;
            const auto id = std::get<1>(*iter);

            for (auto iter2 = seconddata.begin(); iter2 != seconddata.end(); ++iter2) {

                if (std::get<1>(*iter2) == id) {

                    other = std::get<0>(*iter2);
                    break;
                }
            }

            if (!other) {

                // Full search //
                other = secondholder.Find(secondlock, id);
            }

            if (!other)
                continue;

            // Create node if it doesn't exist already //
            if (Nodes.Find(guard, id))
                continue;

            Nodes.ConstructNew(guard, id, *std::get<0>(*iter), *other);
        }

        // And the other way around //
        for (auto iter = seconddata.begin(); iter != seconddata.end(); ++iter) {

            FirstType* other = nullptr;
            const auto id = std::get<1>(*iter);

            for (auto iter2 = firstdata.begin(); iter2 != firstdata.end(); ++iter2) {

                if (std::get<1>(*iter2) == id) {

                    other = std::get<0>(*iter2);
                    break;
                }
            }

            if (!other) {

                // Full search //
                other = firstholder.Find(firstlock, id);
            }

            if (!other)
                continue;

            // Create node if it doesn't exist already //
            if (Nodes.Find(guard, id))
                continue;

            Nodes.ConstructNew(guard, id, *other, *std::get<0>(*iter));
        }
    }
    
public:
    
    HolderType Nodes;
};

//! \brief Base class for systems that use a single component directly
template<class UsedComponent>
class SingleSystem{
public:
    virtual void Run(std::unordered_map<ObjectID, UsedComponent*> &Index,
        GameWorld &world) = 0;
};

}


