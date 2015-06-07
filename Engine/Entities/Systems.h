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

        //! \copydoc System::ProcessNode
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

    //! \brief Sets visibility objects with Ogre nodes that have changed RenderNode::Hidden
    class RenderNodeHiderSystem : public System<RenderNodeHiderNode>{
    public:
        //! \copydoc System::ProcessNode
        //! \pre All systems that can mark Position as updated have been executed
        DLLEXPORT void ProcessNode(RenderNodeHiderNode &node, ObjectID nodesobject,
            NodeHolder<RenderNodeHiderNode> &pool, Lock &poollock) const override
        {
            if(!node._RenderNode.Marked)
                return;

            node._RenderNode.Node->setVisible(!node._RenderNode.Hidden);

            node._RenderNode.Marked = false;
        }

        //! \brief Creates nodes if matching ids are found in all data vectors or
        //! already existing component holders
        //! \note It is more efficient to directly create nodes as entities are created
        DLLEXPORT void CreateNodes(NodeHolder<RenderNodeHiderNode> &nodes,
            const std::vector<std::tuple<RenderNode*, ObjectID>> &firstdata)
        {
            GUARD_LOCK_OTHER((&nodes));

            for(auto iter = firstdata.begin(); iter != firstdata.end(); ++iter){

                // Create node if it doesn't exist already //
                if(nodes.Find(guard, std::get<1>(*iter)))
                    continue;

                try{
                    nodes.ConstructNew(guard, std::get<1>(*iter), *std::get<0>(*iter));
                } catch(const Exception &e){

                    Logger::Get()->Error("CreateNodes: failed to create render node hider node "
                        "for object "+Convert::ToString(std::get<1>(*iter))+", exception: ");
                    e.PrintToLog();
                    
                    continue;
                }
            }
        }
    };


    //! \brief Sends updated entities from server to clients
    //! \todo Change this to take distance into account
    //! don't send as many updates to clients far away
    class SendableSystem : public System<SendableNode>{
    public:

        //! \copydoc System::ProcessNode
        //! \pre Final states for entities have been created for current tick
        //! \param world The calling world, used to find components
        //! \param ticknumber Current tick which will be added to the update packets
        DLLEXPORT void ProcessNode(SendableNode &node, ObjectID nodesobject,
            NodeHolder<SendableNode> &pool, Lock &poollock, GameWorld* world, Lock &worldlock,
            int ticknumber) const;


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

    //! \brief Runs TrackController logic
    //! \todo Create a process node variant for non physics based objects
    class TrackControllerSystem : public System<TrackControllerNode>{
    public:
        //! \copydoc System::ProcessNode
        //! \post Physics can be run for this timestep as all track controlled objects should
        //! have valid applyforces after this
        //! \param timestep The time step in seconds
        DLLEXPORT void ProcessNode(TrackControllerNode &node, ObjectID nodesobject,
            NodeHolder<TrackControllerNode> &pool, Lock &poollock, float timestep,
            GameWorld* world, Lock &worldlock) const;
        
        //! \brief Creates nodes if matching ids are found in all data vectors or
        //! already existing component holders
        //! \note It is more efficient to directly create nodes as entities are created
        template<class FirstType, class SecondType, class ThirdType>
        DLLEXPORT void CreateNodes(NodeHolder<TrackControllerNode> &nodes,
            const std::vector<std::tuple<FirstType*, ObjectID>> &firstdata,
            const std::vector<std::tuple<SecondType*, ObjectID>> &seconddata,
            const ComponentHolder<SecondType> &secondholder, Lock &secondlock,
            const std::vector<std::tuple<SecondType*, ObjectID>> &thirddata,
            const ComponentHolder<SecondType> &thirdholder, Lock &thirdlock)
        {

            GUARD_LOCK_OTHER((&nodes));

            for(auto iter = firstdata.begin(); iter != firstdata.end(); ++iter){

                SecondType* second = nullptr;
                
                for(auto iter2 = seconddata.begin(); iter2 != seconddata.end(); ++iter2){

                    if(std::get<1>(*iter2) == std::get<1>(*iter)){

                        second = std::get<0>(*iter2);
                        break;
                    }
                }

                if(!second){

                    // Full search //
                    second = secondholder.Find(secondlock, std::get<1>(*iter));
                }

                if(!second)
                    continue;

                ThirdType* third = nullptr;
                
                for(auto iter2 = thirddata.begin(); iter2 != thirddata.end(); ++iter2){

                    if(std::get<1>(*iter2) == std::get<1>(*iter)){

                        third = std::get<0>(*iter2);
                        break;
                    }
                }                

                if(!third)
                    continue;

                // Create node if it doesn't exist already //
                if(nodes.Find(guard, std::get<1>(*iter)))
                    continue;

                TrackControllerNode::Parameters params;
                params.Set(*second);
                params.Set(*third);
                params.Set(*std::get<0>(*iter));

                try{
                    nodes.ConstructNew(guard, std::get<1>(*iter), params);
                } catch(const Exception &e){

                    Logger::Get()->Error("CreateNodes: failed to create track controller node "
                        "for object "+Convert::ToString(std::get<1>(*iter))+", exception: ");
                    e.PrintToLog();
                    
                    continue;
                }
            }
        }
    };


    //! \brief Interpolates positions between states of marked Received objects
	class ReceivedPositionSystem : public System<ReceivedPosition>{
	public:
        
        //! \copydoc System::ProcessNode
        //! \pre All systems that can mark Position as updated have been executed
        DLLEXPORT void ProcessNode(ReceivedPosition &node, ObjectID nodesobject,
            NodeHolder<ReceivedPosition> &pool, Lock &poollock, int tick, float progress) const
        {
            // Unmarked nodes should have invalid interpolation status
            if(!node._Received.Marked)
                return;

            float adjustedprogress = progress;
            const Received::StoredState* first;
            const Received::StoredState* second;

            GUARD_LOCK_OTHER((&node._Received));

            try{
                node._Received.GetServerSentStates(guard, &first, &second, tick, adjustedprogress);

            } catch(const InvalidState&){

                // If not found unmark to avoid running unneeded //
                node._Received.Marked = false;
                return;
            }

            switch(node._Received.SendableHandleType){
                case SENDABLE_TYPE_PROP:
                case SENDABLE_TYPE_BRUSH:
                {
                    node._Position.Interpolate(*reinterpret_cast<const PositionDeltaState*>(
                            first->DirectData),
                        *reinterpret_cast<const PositionDeltaState*>(second->DirectData),
                        adjustedprogress);
                }
                break;
                default:
                {
                    Logger::Get()->Error("ReceivedPositionSystem: trying to handle received "
                        "whose type is not properly added");
                    DEBUG_BREAK;
                    return;
                }
            }
        }

        //! \brief Creates nodes if matching ids are found in all data vectors or
        //! already existing component holders
        //! \note It is more efficient to directly create nodes as entities are created
        template<class FirstType, class SecondType>
        DLLEXPORT void CreateNodes(NodeHolder<ReceivedPosition> &nodes,
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

                    Logger::Get()->Error("CreateNodes: failed to create received position node "
                        "for object "+Convert::ToString(std::get<1>(*iter))+", exception: ");
                    e.PrintToLog();
                    
                    continue;
                }
            }
        }
    };
}
