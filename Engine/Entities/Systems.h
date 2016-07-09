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
class RenderingPositionSystem : public System<std::tuple<RenderNode&, Position&>>{
    
    void ProcessNode(std::tuple<RenderNode&, Position&> &node, ObjectID id)
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

    void Clear(){

        Nodes.Clear();
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
            const ComponentHolder<FirstType> &firstholder, Lock &firstlock,
            const ComponentHolder<SecondType> &secondholder, Lock &secondlock)
    {
        static_assert(std::is_same<FirstType, RenderNode>::value, 
            "CreateNodes FirstType is incorrect");
        TupleNodeHelper(Nodes, firstdata, seconddata, firstholder, firstlock, secondholder, 
            secondlock);
    }

 private:

    ObjectPool<std::tuple<RenderNode&, Position&>, ObjectID> Nodes;
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

    DLLEXPORT void Run(std::unordered_map<ObjectID, Received*> &Index, GameWorld &world)
        override;
};
}
