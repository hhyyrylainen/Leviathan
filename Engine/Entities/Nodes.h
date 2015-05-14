#pragma once

//! \file Contains the common nodes that are used by internal systems

// ------------------------------------ //
#include "Include.h"

#include "Node.h"
#include "Components.h"

namespace Leviathan{

    //! \brief Updates position of render node
	class RenderingPosition : public Node{
	public:

        DLLEXPORT RenderingPosition(Position& pos, RenderNode& node);

        DLLEXPORT RenderingPosition(RenderNode& node, Position& pos);


        Position& _Position;
        RenderNode& _RenderNode;
	};

    //! \brief Holds dirty flag for quikcly looping sendable entities
    class SendableNode : public Node{
    public:

        DLLEXPORT SendableNode(Sendable &sendable);

        
        Sendable& _Sendable;
    };
}
