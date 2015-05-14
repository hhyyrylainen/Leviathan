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

        //! \brief Creates nodes if matching ids are found in all data vectors or
        //! already existing component holders
        //! \note It is more efficient to directly create nodes as entities are created
        template<class FirstType, class SecondType>
        DLLEXPORT void CreateNodes(NodeHolder<RenderingPosition> &nodes,
            const std::vector<std::tuple<FirstType*, ObjectID>> &firstdata,
            const std::vector<std::tuple<SecondType*, ObjectID>> &seconddata,
            const ComponentHolder<SecondType> &secondholder, Lock &secondlock)
        {

            GUARD_LOCK_OTHER((&nodes));

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
                if(nodes.Find(guard, std::get<1>(*iter)))
                    continue;

                try{
                    nodes.ConstructNew(guard, std::get<1>(*iter), *other, *std::get<0>(*iter));
                } catch(const Exception &e){

                    Logger::Get()->Error("CreateNodes: failed to create render position node "
                        "for object "+Convert::ToString(std::get<1>(*iter))+", exception: ");
                    e.PrintToLog();
                    
                    continue;
                }
            }
        }
    };


    //! \brief Sends updated entities from server to clients
    class SendableSystem : public System<SendableNode>{
    public:

        //! \pre Final states for entities have been created for current tick
        DLLEXPORT void ProcessNode(SendableNode &node, ObjectID nodesobject,
            NodeHolder<SendableNode> &pool, Lock &poollock) const override
        {
            if(!node._Sendable.Marked)
                return;

            // Send updates //
            DEBUG_BREAK;
            
            node._Sendable.Marked = false;
        }


        //! \brief Creates nodes if matching ids are found in all data vectors or
        //! already existing component holders
        //! \note It is more efficient to directly create nodes as entities are created
        DLLEXPORT void CreateNodes(NodeHolder<SendableNode> &nodes,
            const std::vector<std::tuple<Sendable*, ObjectID>> &data)
        {

            GUARD_LOCK_OTHER((&nodes));

            for(auto iter = data.begin(); iter != data.end(); ++iter){

                // Create node if it doesn't exist already //
                if(nodes.Find(guard, std::get<1>(*iter)))
                    continue;

                try{
                    nodes.ConstructNew(guard, std::get<1>(*iter), *std::get<0>(*iter));
                } catch(const Exception &e){

                    Logger::Get()->Error("CreateNodes: failed to create sendable node for object "+
                        Convert::ToString(std::get<1>(*iter))+", exception: ");
                    e.PrintToLog();
                    
                    continue;
                }
            } 
        }
    };
    
    
}
