#pragma once

//! \file Contains all common systems that GameWorld will run on its components at specified times

// ------------------------------------ //
#include "Include.h"

#include "System.h"
#include "Nodes.h"
#include "EntityCommon.h"
#include "../Utility/Convert.h"

#include "OgreSceneNode.h"

namespace Leviathan{

    //! \brief Moves nodes of entities that have their positions changed
	class RenderingPositionSystem : public System<RenderingPosition>{
	public:

        //! \pre All systems that can mark Position as updated have been executed
        DLLEXPORT void ProcessNode(RenderingPosition &node, ObjectID nodesobject,
            NodeHolder<RenderingPosition> &pool, Lock &poollock) const override
        {
            // We need to have a guarantee that no other system that marks or unmarks positions
            // is being run at the same time
            if(!node._Position.Marked)
                return;

            node._RenderNode.Node->setPosition(node._Position._Position);
            node._RenderNode.Node->setOrientation(node._Position._Orientation);

            node._Position.Marked = false;
        }

        //! \brief Creates nodes if matching ids are found in all argument vectors
        //! \note It is more efficient to directly create nodes as
        DLLEXPORT void CreateNodes(NodeHolder<RenderingPosition> &nodes,
            const std::vector<std::tuple<Position*, ObjectID>> &positions,
            const ComponentHolder<Position> &positionholder,
            const std::vector<std::tuple<RenderNode*, ObjectID>> &rendernodes,
            const ComponentHolder<RenderNode> &rendernodeholder)
        {

            GUARD_LOCK_OTHER((&nodes));

            // This needs to be done both ways because components could be created at different times //
            _CreateNodesSingle(nodes, guard, positions, rendernodes, rendernodeholder);
            _CreateNodesSingle(nodes, guard, rendernodes, positions, positionholder);
        }

    protected:

        //! Single way search helper for CreateNodes
        template<class FirstType, class SecondType>
        void _CreateNodesSingle(NodeHolder<RenderingPosition> &nodes, Lock &nodeguard,
            const std::vector<std::tuple<FirstType*, ObjectID>> &firstdata,
            const std::vector<std::tuple<SecondType*, ObjectID>> &seconddata,
            const ComponentHolder<SecondType> &secondholder)
        {

            for(auto iter = firstdata.begin(); iter != firstdata.end(); ++iter){

                SecondType* other = nullptr;
                
                for(auto iter2 = seconddata.begin(); iter != seconddata.end(); ++iter2){

                    if(iter2->get<1>() == iter->get<1>()){

                        other = iter2->get<0>();
                        break;
                    }
                }

                if(!other){

                    // Full search //
                    other = secondholder.Find(iter->get<1>());
                }

                if(!other)
                    continue;

                // Create node if it doesn't exist already //
                if(nodes.Find(nodeguard, iter->get<1>()))
                    continue;

                try{
                    nodes.ConstructNew(nodeguard, iter->get<1>(), *other, *iter->get<0>());
                } catch(const Exception &e){

                    Logger::Get()->Error("CreateNodes: failed to create node for object "+
                        Convert::ToString(iter->get<1>())+", exception: ");
                    e.PrintToLog();
                    
                    continue;
                }
            }
        }
        
	};

    //! \brief Sends updated entities from server to clients
    class SendableSystem : public System<SendableNode>{

        //! \pre Final states for entities have been created for current tick
        DLLEXPORT void ProcessNode(SendableNode &node, ObjectID nodesobject,
            NodeHolder<SendableNode> &pool, Lock &poollock) const override
        {
            if(!node.IsDirty)
                return;

            
            
            node.IsDirty = false;
        }        
    };
    
    
}
