// ------------------------------------ //
#include "Nodes.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

// ------------------ RenderingPosition ------------------ //
DLLEXPORT RenderingPosition::RenderingPosition(Position& pos, RenderNode& node) :
    _Position(pos), _RenderNode(node)
{
    
}

DLLEXPORT RenderingPosition::RenderingPosition(RenderNode& node, Position& pos) :
    _Position(pos), _RenderNode(node)
{

}
// ------------------ SendableNode ------------------ //
DLLEXPORT SendableNode::SendableNode(Sendable &sendable) : _Sendable(sendable){

}
// ------------------ TrackControllerNode ------------------ //
DLLEXPORT TrackControllerNode::TrackControllerNode(const Parameters &params) :
    _TrackController(*params._TrackController), _PositionMarkerOwner(*params._PositionMarkerOwner),
    _Parent(*params._Parent)
{

}
// ------------------ ReceivedPosition ------------------ //
DLLEXPORT ReceivedPosition::ReceivedPosition(Received &received, Position& pos) :
    _Received(received), _Position(pos)
{

}

DLLEXPORT ReceivedPosition::ReceivedPosition(Position& pos, Received &received) :
    _Received(received), _Position(pos)
{

}
