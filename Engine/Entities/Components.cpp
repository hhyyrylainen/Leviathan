// ------------------------------------ //
#include "Components.h"

#include "OgreSceneManager.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

// ------------------ Position ------------------ //
DLLEXPORT Position::Position(){

}

DLLEXPORT bool Position::Init(const Float3 pos, const Float4 rot){

    _Position = pos;
    _Orientation = rot;

    Marked = true;
}

//! \brief Initializes at 0, 0, 0
DLLEXPORT bool Position::Init(){

    _Position = Float3(0, 0, 0);
    _Orientation = Float4::IdentityQuaternion();
}
// ------------------------------------ //
DLLEXPORT void Position::ApplyPositionData(const PositionData &data){

    _Position = data._Position;
    _Orientation = data._Orientation;

    Marked = true;
}

DLLEXPORT void Position::LoadDataFromPacket(sf::Packet &packet, PositionData &data){

    packet >> data._Position >> data._Orientation;

    if(!packet)
        throw InvalidArgument("Packet has invalid format");
}
// ------------------------------------ //
DLLEXPORT void Position::AddDataToPacket(sf::Packet &packet) const{

    packet << _Position << _Orientation;
}

DLLEXPORT void Position::ApplyDataFromPacket(sf::Packet &packet){

    packet >> _Position >> _Orientation;

    if(!packet)
        throw InvalidArgument("Packet has invalid format");

    Marked = true;
}
// ------------------------------------ //
DLLEXPORT void Position::Interpolate(PositionDeltaState &from, PositionDeltaState &to,
    float progress)
{
    Float3 pos = _Position;
    Float4 rot = _Orientation;

    // Position
    if(second.ValidFields & PRDELTAUPDATED_POS_X){

        if(first.ValidFields & PRDELTAUPDATED_POS_X){
            
            pos.X = first.Position.X + (second.Position.X-first.Position.X)*progress;
        } else {

            pos.X = second.Position.X;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_POS_X){
        
        pos.X = first.Position.X;
    }

    if(second.ValidFields & PRDELTAUPDATED_POS_Y){

        if(first.ValidFields & PRDELTAUPDATED_POS_Y){
            
            pos.Y = first.Position.Y + (second.Position.Y-first.Position.Y)*progress;
        } else {

            pos.Y = second.Position.Y;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_POS_Y){
        
        pos.Y = first.Position.Y;
    }
    
    if(second.ValidFields & PRDELTAUPDATED_POS_Z){

        if(first.ValidFields & PRDELTAUPDATED_POS_Z){
            
            pos.Z = first.Position.Z + (second.Position.Z-first.Position.Z)*progress;
        } else {

            pos.Z = second.Position.Z;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_POS_Z){
        
        pos.Z = first.Position.Z;
    }
    
    // Rotation
    // TODO: spherical interpolation for rotation
    if(second.ValidFields & PRDELTAUPDATED_ROT_X){

        if(first.ValidFields & PRDELTAUPDATED_ROT_X){
            
            rot.X = first.Rotation.X + (second.Rotation.X-first.Rotation.X)*progress;
        } else {

            rot.X = second.Rotation.X;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_ROT_X){

        rot.X = first.Rotation.X;
    }

    if(second.ValidFields & PRDELTAUPDATED_ROT_Y){

        if(first.ValidFields & PRDELTAUPDATED_ROT_Y){
            
            rot.Y = first.Rotation.Y + (second.Rotation.Y-first.Rotation.Y)*progress;
        } else {

            rot.Y = second.Rotation.Y;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_ROT_Y){

        rot.Y = first.Rotation.Y;
    }
    
    if(second.ValidFields & PRDELTAUPDATED_ROT_Z){

        if(first.ValidFields & PRDELTAUPDATED_ROT_Z){
            
            rot.Z = first.Rotation.Z + (second.Rotation.Z-first.Rotation.Z)*progress;
        } else {

            rot.Z = second.Rotation.Z;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_ROT_Z){

        rot.Z = first.Rotation.Z;
    }
    
    if(second.ValidFields & PRDELTAUPDATED_ROT_W){

        if(first.ValidFields & PRDELTAUPDATED_ROT_W){
            
            rot.W = first.Rotation.W + (second.Rotation.W-first.Rotation.W)*progress;
        } else {

            rot.W = second.Rotation.W;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_ROT_W){

        rot.W = first.Rotation.W;
    }
    
    _Position = pos;
    _Orientation = rot;

    Marked = true;
}
// ------------------ RenderNode ------------------ //
DLLEXPORT RenderNode::RenderNode(){

}

DLLEXPORT bool RenderNode::Init(){

    Node = nullptr;
}

DLLEXPORT void RenderNode::Release(Ogre::Scene* worldsscene){

    worldsscene->destroySceneNode(Node);
    Node = nullptr;
}



