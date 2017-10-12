// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once

//! \file Contains all common systems that GameWorld will run on its components
//! at specified times

// ------------------------------------ //
#include "Include.h"

#include "System.h"
#include "Components.h"
#include "StateInterpolator.h"

#include "Utility/Convert.h"

#include "Generated/ComponentStates.h"

#include "OgreSceneNode.h"

namespace Leviathan{

// ------------------------------------ //
// State creation systems
class PositionStateSystem : public StateCreationSystem<Position, PositionState>{

};


// ------------------------------------ //

//! \brief Moves nodes of entities that have their positions changed
class RenderingPositionSystem : public System<std::tuple<RenderNode&, Position&>>{
    
    void ProcessNode(std::tuple<RenderNode&, Position&> &node, ObjectID id,
        const StateHolder<PositionState> &heldstates, int tick, int timeintick)
    {
        auto& pos = std::get<1>(node);
        
        if(!pos.StateMarked)
            return;

        auto interpolated = StateInterpolator::Interpolate(heldstates, id, &pos,
            tick, timeintick);

        auto& rendernode = std::get<0>(node);

        if(!std::get<0>(interpolated)){
            // No states to interpolate //
            rendernode.Node->setPosition(pos.Members._Position);
            rendernode.Node->setOrientation(pos.Members._Orientation);
            return;
        }

        const auto& state = std::get<1>(interpolated);
        rendernode.Node->setPosition(state._Position);
        rendernode.Node->setOrientation(state._Orientation);
    }
    
public:
    template<class GameWorldT>
        void Run(GameWorldT &world, const StateHolder<PositionState> &heldstates, int tick,
            int timeintick)
    {
        auto& index = Nodes.GetIndex();
        for(auto iter = index.begin(); iter != index.end(); ++iter){

            this->ProcessNode(*iter->second, iter->first, heldstates, tick, timeintick);
        }
    }

    //! \brief Creates nodes if matching ids are found in all data vectors or
    //! already existing component holders
    //!
    //! This should be 
    //! \note It is more efficient to directly create nodes as entities are created
    template<class FirstType, class SecondType>
        void CreateNodes(
            const std::vector<std::tuple<FirstType*, ObjectID>> &firstdata,
            const std::vector<std::tuple<SecondType*, ObjectID>> &seconddata,
            const ComponentHolder<FirstType> &firstholder,
            const ComponentHolder<SecondType> &secondholder)
    {
        static_assert(std::is_same<FirstType, RenderNode>::value, 
            "CreateNodes FirstType is incorrect");
        TupleNodeHelper(Nodes, firstdata, seconddata, firstholder, secondholder);
    }

    //! \brief Creates nodes if matching ids are found in all data vectors or
    //! already existing component holders
    //!
    //! This should be 
    //! \note It is more efficient to directly create nodes as entities are created
    template<class FirstType, class SecondType>
        void DestroyNodes(
            const std::vector<std::tuple<FirstType*, ObjectID>> &firstdata,
            const std::vector<std::tuple<SecondType*, ObjectID>> &seconddata)
    {
        Nodes.RemoveBasedOnKeyTupleList(firstdata);
        Nodes.RemoveBasedOnKeyTupleList(seconddata);
    }    
};

//! \brief Sets visibility objects with Ogre nodes that have changed RenderNode::Hidden
class RenderNodeHiderSystem : public SingleSystem<RenderNode>{
public:

    void Run(GameWorld &world, std::unordered_map<ObjectID, RenderNode*> &index){

        for(auto iter = index.begin(); iter != index.end(); ++iter){

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
    void Run(std::unordered_map<ObjectID, Sendable*> &index, GameWorld &world){

        for(auto iter = index.begin(); iter != index.end(); ++iter){

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

    DLLEXPORT void Run(std::unordered_map<ObjectID, Received*> &index, GameWorld &world);
};
}
