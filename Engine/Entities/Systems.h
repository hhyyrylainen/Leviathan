#pragma once

//! \file Contains all common systems that GameWorld will run on its components at specified times

// ------------------------------------ //
#include "Include.h"

#include "System.h"
#include "Nodes.h"
#include "EntityCommon.h"

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
