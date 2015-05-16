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


// ------------------ Physics ------------------ //
DLLEXPORT void Physics::JumpTo(Position &target){

    GUARD_LOCK();
    SetPosition(guard, target._Position, target._Orientation);
}

DLLEXPORT bool Physics::SetPosition(Lock &guard, const Float3 &pos, const Float4 &orientation){

    if(!Body)
        return false;
    
    Ogre::Matrix4 matrix;
    matrix.makeTransform(pos, Float3(1, 1, 1), orientation);

    Ogre::Matrix4 tmatrix = matrix.transpose();

    // Update body //
    NewtonBodySetMatrix(Body, &tmatrix[0][0]);
}


void Leviathan::Entity::Prop::PhysicsMovedEvent(const NewtonBody* const body,
    const dFloat* const matrix, int threadIndex)
{

	// first create Ogre 4x4 matrix from the matrix //
	Ogre::Matrix4 mat(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6],
        matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13],
        matrix[14], matrix[15]);

	// needs to convert from d3d style matrix to OpenGL style matrix //
    // TODO: do this transpose in the mat constructor
	Ogre::Matrix4 tmat = mat.transpose();

	Physics* tmp = reinterpret_cast<Physics*>(NewtonBodyGetUserData(body));

    // The object needs to be locked here //
    GUARD_LOCK_OTHER(tmp);
    
    if(tmp->UpdatePosition){

        GUARD_LOCK_OTHER_NAME(tmp->UpdatePosition, guard2);

        tmp->UpdatePosition->_Position = tmat.getTrans();
        tmp->UpdatePosition->_Orientation = tmat.extractQuaternion();
        tmp->UpdatePosition->Marked = true;
    }
    
    if(tmp->UpdateSendable){
        
        tmp->UpdateSendable = true;
    }
    
    tmp->Marked = true;
}


// ------------------ TrackController ------------------ //

DLLEXPORT bool TrackEntityController::SetStateToInterpolated(ObjectDeltaStateData &first,
    ObjectDeltaStateData &second, float progress)
{

    const TrackControllerState& from = static_cast<const TrackControllerState&>(first);
    const TrackControllerState& to = static_cast<const TrackControllerState&>(second);

    if(progress < 0.f)
        progress = 0.f;

    if(progress > 1.f)
        progress = 1.f;
    
    GUARD_LOCK();

    if(to.ValidFields & TRACKSTATE_UPDATED_SPEED){

        ChangeSpeed = from.ChangeSpeed*(1.f-progress) + progress*to.ChangeSpeed;
        
    } else {

        ChangeSpeed = from.ChangeSpeed;
    }

    if(to.ValidFields & TRACKSTATE_UPDATED_NODE && from.ReachedNode != to.ReachedNode){

        // Node has changed //
        const float fromtotalvalue = from.ReachedNode+from.NodeProgress;

        const float tototalvalue = to.ReachedNode +
            (to.ValidFields & TRACKSTATE_UPDATED_PROGRESS ? to.NodeProgress : 0);

        const float mixed = fromtotalvalue*(1.f-progress) + tototalvalue*progress;

        ReachedNode = floor(mixed);
        NodeProgress = mixed-ReachedNode;
        
    } else {

        ReachedNode = from.ReachedNode;

        if(to.ValidFields & TRACKSTATE_UPDATED_PROGRESS){
            
            NodeProgress = from.NodeProgress*(1.f-progress) + progress*to.NodeProgress;
            
        } else {

            NodeProgress = from.NodeProgress;
        }
    }

    _SanityCheckNodeProgress(guard);
    
    return true;
}
