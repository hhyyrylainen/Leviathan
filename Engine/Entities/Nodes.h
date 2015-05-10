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

        DLLEXPORT bool Init(Position& pos, RenderNode& node);


        Position& _Position;
        RenderNode& _RenderNode;
	};

    //! \brief Holds dirty flag for quikcly looping sendable entities
    class SendableNode : public Node{
    public:

        DLLEXPORT bool Init();

        bool IsDirty;
    };
}
