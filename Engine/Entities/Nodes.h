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

    //! \brief TrackController for physics world update
    class TrackControllerNode : public Node{
    public:
        struct Parameters{

            inline void Set(TrackController& toset){

                _TrackController = &toset;
            }
            
            inline void Set(PositionMarkerOwner& toset){

                _PositionMarkerOwner = &toset;
            }
            
            inline void Set(Parent& toset){

                _Parent = &toset;
            }
            
            TrackController* _TrackController;
            PositionMarkerOwner* _PositionMarkerOwner;
            Parent* _Parent;
        };

        DLLEXPORT TrackControllerNode(const Parameters &params);


        TrackController& _TrackController;
        PositionMarkerOwner& _PositionMarkerOwner;
        Parent& _Parent;
    };

    //! \brief Interpolates position of Received object
	class ReceivedPosition : public Node{
	public:

        DLLEXPORT ReceivedPosition(Received &received, Position& pos);

        DLLEXPORT ReceivedPosition(Position& pos, Received &received);

        
        Received& _Received;
        Position& _Position;
	};
}
