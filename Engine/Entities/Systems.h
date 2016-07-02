// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once

//! \file Contains all common systems that GameWorld will run on its components
//! at specified times

// ------------------------------------ //
#include "Include.h"

#include "System.h"
#include "Components.h"
#include "Utility/Convert.h"

#include "OgreSceneNode.h"

namespace Leviathan{

//! \brief Moves nodes of entities that have their positions changed
class RenderingPositionSystem : public System<std::tuple<RenderNode, Position>>{
    
    void ProcessNode(std::tuple<RenderNode, Position> &node, ObjectID id)
    {
        auto& pos = std::get<1>(node);
        if(!pos.Marked)
            return;

        auto& rendernode = std::get<0>(node);
        rendernode.Node->setPosition(pos.Members._Position);
        rendernode.Node->setOrientation(pos.Members._Orientation);
    }
    
 public:

    void Run(GameWorld &world) override{

        RunAllNodes<RenderingPositionSystem, &RenderingPositionSystem::ProcessNode>(*this);
    }

    //! \brief Creates nodes if matching ids are found in all data vectors or
    //! already existing component holders
    //! \note It is more efficient to directly create nodes as entities are created
    template<class FirstType, class SecondType>
        void CreateNodes(const std::vector<std::tuple<FirstType*, ObjectID>> &firstdata,
            const std::vector<std::tuple<SecondType*, ObjectID>> &seconddata,
            const ComponentHolder<SecondType> &secondholder, Lock &secondlock)
    {

        GUARD_LOCK_OTHER(Nodes);

        for(auto iter = firstdata.begin(); iter != firstdata.end(); ++iter){

            SecondType* other = nullptr;
                
            for(auto iter2 = seconddata.begin(); iter2 != seconddata.end(); ++iter2){

                if(std::get<1>(*iter2) == std::get<1>(*iter)){

                    other = std::get<0>(*iter2);
                    break;
                }
            }

            if(!other){

                // Full search //
                other = secondholder.Find(secondlock, std::get<1>(*iter));
            }

            if(!other)
                continue;

            // Create node if it doesn't exist already //
            if(Nodes.Find(guard, std::get<1>(*iter)))
                continue;

            try{
                Nodes.ConstructNew(guard, std::get<1>(*iter), *other, *std::get<0>(*iter));
            } catch(const Exception &e){

                Logger::Get()->Error("CreateNodes: failed to create render position node "
                    "for object "+Convert::ToString(std::get<1>(*iter))+", exception: ");
                e.PrintToLog();
                    
                continue;
            }
        }
    }
};

//! \brief Sets visibility objects with Ogre nodes that have changed RenderNode::Hidden
class RenderNodeHiderSystem : public SingleSystem<RenderNode>{
public:

    void Run(std::unordered_map<ObjectID, RenderNode*> &Index, GameWorld &world) override{

        for(auto iter = Index.begin(); iter != Index.end(); ++iter){

            auto& node = *iter->second;
            
            if(!node.Marked)
                return;

            node.Node->setVisible(!node.Hidden);

            node.Marked = false;
        }
    }
};


//! \brief Sends updated entities from server to clients
//! \todo Change this to take distance into account
//! don't send as many updates to clients far away
class SendableSystem : public SingleSystem<Sendable>{
public:

    //! \pre Final states for entities have been created for current tick
    void Run(std::unordered_map<ObjectID, Sendable*> &Index, GameWorld &world) override{

        for(auto iter = Index.begin(); iter != Index.end(); ++iter){

            auto& node = *iter->second;
            
            if(!node.Marked)
                return;

            HandleNode(iter->first, node, world);

            node.Marked = false;
        }
    }

protected:

    DLLEXPORT void HandleNode(ObjectID id, Sendable &obj, GameWorld &world);
};

//! \brief Interpolates states for received objects and handles locally controlled entities
class ReceivedSystem : public SingleSystem<Received>{
public:

    void Run(std::unordered_map<ObjectID, Received*> &Index, GameWorld &world) override{

        const float progress = world.GetTickProgress();
        const auto tick = world.GetTickNumber();
        
        for(auto iter = Index.begin(); iter != Index.end(); ++iter){

            auto& node = *iter->second;

            // Unmarked nodes should have invalid interpolation status
            if(!node.Marked)
                return;
            
            if(!node.LocallyControlled){

                // Interpolate received states //
                float adjustedprogress = progress;
                const Received::StoredState* first;
                const Received::StoredState* second;

                try{
                    node.GetServerSentStates(&first, &second, tick, adjustedprogress);
                } catch(const InvalidState&){

                    // If not found unmark to avoid running unneeded //
                    node.Marked = false;
                    continue;
                }
                
                first->Interpolate(second, adjustedprogress);
                
            } else {

                // Send updates to the server //
                
            }
        }
    }
};
}
