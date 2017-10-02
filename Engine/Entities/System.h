// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"

#include "Component.h"
#include "EntityCommon.h"
#include "StateHolder.h"

#include "Common/ObjectPool.h"
#include "Exceptions.h"


namespace Leviathan{

template<class NodeType>
class NodeHolder : public ObjectPool<NodeType, ObjectID>{};

//! \brief Base for all entity component related systems
//!
//! For ones that use nodes. Not for ones that directly use a single component type
template<class UsedNode>
class System {
public:

    using HolderType = NodeHolder<UsedNode>;

    // Example run method
    // void Run(GameWorld &world);

    void Clear(){

        Nodes.Clear();
    }

    auto GetNodeCount() const{
        return Nodes.GetObjectCount();
    }    

protected:

    /* Template for node run method, copy-paste and fill in the parameters
        
        auto& index = Nodes.GetIndex();
        for (auto iter = index.begin(); iter != index.end(); ++iter) {

            this->ProcessNode(*iter->second, iter->first, );
        }
    */

    //! \brief Helper function for creating nodes based on std::tuple 
    template<class FirstType, class SecondType>
    static void TupleNodeHelper(
        ObjectPool<std::tuple<FirstType&, SecondType&>, ObjectID> &Nodes,
        const std::vector<std::tuple<FirstType*, ObjectID>> &firstdata,
        const std::vector<std::tuple<SecondType*, ObjectID>> &seconddata,
        const ComponentHolder<FirstType> &firstholder,
        const ComponentHolder<SecondType> &secondholder) 
    {
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
                other = secondholder.Find(id);
            }

            if (!other)
                continue;

            // Create node if it doesn't exist already //
            if (Nodes.Find(id))
                continue;

            Nodes.ConstructNew(id, *std::get<0>(*iter), *other);
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
                other = firstholder.Find(id);
            }

            if (!other)
                continue;

            // Create node if it doesn't exist already //
            if (Nodes.Find(id))
                continue;

            Nodes.ConstructNew(id, *other, *std::get<0>(*iter));
        }
    }
    
public:
    
    HolderType Nodes;
};

//! \brief Base class for systems that use a single component directly
template<class UsedComponent>
class SingleSystem{
public:
    // Example run method
    //void Run(std::unordered_map<ObjectID, UsedComponent*> &Index, GameWorld &world);
};

//! \brief Base class for all systems that create states from changed components
template<class UsedComponent, class ComponentState>
class StateCreationSystem{
public:
    void Run(GameWorld &world, std::unordered_map<ObjectID, UsedComponent*> &index,
        StateHolder<ComponentState> &heldstates, int worldtick)
    {
        for(auto iter = index.begin(); iter != index.end(); ++iter){

            auto& node = *iter->second;
            
            if(!node.Marked)
                return;

            // Needs a new state //
            heldstates.CreateStateIfChanged(iter->first, node, worldtick);

            node.Marked = false;
        }
    }
};


}


