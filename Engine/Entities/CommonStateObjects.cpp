// ------------------------------------ //
#ifndef LEVIATHAN_COMMONSTATEOBJECTS
#include "CommonStateObjects.h"
#endif
#include "Exceptions.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT ObjectDeltaStateData::ObjectDeltaStateData(int tick) : Tick(tick){

}

DLLEXPORT ObjectDeltaStateData::~ObjectDeltaStateData(){

}
// ------------------ PhysicalDeltaState ------------------ //
DLLEXPORT Leviathan::PhysicalDeltaState::PhysicalDeltaState(int tick, const Float3 &position,
    const Float4 &rotation, const Float3 &velocity, const Float3 &torque) :
    ObjectDeltaStateData(tick),
    Position(position), Rotation(rotation), Velocity(velocity), Torque(torque),
    ValidFields(PPDELTA_ALL_UPDATED)
{

}

DLLEXPORT Leviathan::PhysicalDeltaState::PhysicalDeltaState(int tick,
    sf::Packet &packet) :
    ObjectDeltaStateData(tick)
{
    
    // We need to do the opposite of what we do in CreateUpdatePacket //
    packet >> ValidFields;

    if(!packet)
        throw InvalidArgument("invalid packet for positionable delta state");
    
    // Position
    if(ValidFields & PPDELTAUPDATED_POS_X)
        packet >> Position.X;

    if(ValidFields & PPDELTAUPDATED_POS_Y)
        packet >> Position.Y;
    
    if(ValidFields & PPDELTAUPDATED_POS_Z)
        packet >> Position.Z;

    // Rotation
    if(ValidFields & PPDELTAUPDATED_ROT_X)
        packet >> Rotation.X;

    if(ValidFields & PPDELTAUPDATED_ROT_Y)
        packet >> Rotation.Y;
    
    if(ValidFields & PPDELTAUPDATED_ROT_Z)
        packet >> Rotation.Z;
    
    if(ValidFields & PPDELTAUPDATED_ROT_W)
        packet >> Rotation.W;

    // Velocity
    if(ValidFields & PPDELTAUPDATED_VEL_X)
        packet >> Velocity.X;

    if(ValidFields & PPDELTAUPDATED_VEL_Y)
        packet >> Velocity.Y;
    
    if(ValidFields & PPDELTAUPDATED_VEL_Z)
        packet >> Velocity.Z;

    // Torque
    if(ValidFields & PPDELTAUPDATED_TOR_X)
        packet >> Torque.X;

    if(ValidFields & PPDELTAUPDATED_TOR_Y)
        packet >> Torque.Y;
    
    if(ValidFields & PPDELTAUPDATED_TOR_Z)
        packet >> Torque.Z;

    if(!packet)
        throw InvalidArgument("invalid packet for positionable delta state");
}

DLLEXPORT Leviathan::PhysicalDeltaState::~PhysicalDeltaState(){


}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalDeltaState::CreateUpdatePacket(ObjectDeltaStateData* olderstate,
    sf::Packet &packet)
{

    ValidFields = 0;

    // Check which parts have changed //
    if(!olderstate){

        // When comparing against NULL state everything is updated //
        ValidFields = PPDELTA_ALL_UPDATED;
        
    } else {

        PhysicalDeltaState* other = static_cast<PhysicalDeltaState*>(olderstate);

        // Position
        if(Position.X != other->Position.X)
            ValidFields |= PPDELTAUPDATED_POS_X;

        if(Position.Y != other->Position.Y)
            ValidFields |= PPDELTAUPDATED_POS_Y;

        if(Position.Z != other->Position.Z)
            ValidFields |= PPDELTAUPDATED_POS_Z;

        // Rotation
        if(Rotation.X != other->Rotation.X)
            ValidFields |= PPDELTAUPDATED_ROT_X;

        if(Rotation.Y != other->Rotation.Y)
            ValidFields |= PPDELTAUPDATED_ROT_Y;

        if(Rotation.Z != other->Rotation.Z)
            ValidFields |= PPDELTAUPDATED_ROT_Z;

        if(Rotation.W != other->Rotation.W)
            ValidFields |= PPDELTAUPDATED_ROT_W;

        // Velocity
        if(Velocity.X != other->Velocity.X)
            ValidFields |= PPDELTAUPDATED_VEL_X;

        if(Velocity.Y != other->Velocity.Y)
            ValidFields |= PPDELTAUPDATED_VEL_Y;

        if(Velocity.Z != other->Velocity.Z)
            ValidFields |= PPDELTAUPDATED_VEL_Z;


        // Torque
        if(Torque.X != other->Torque.X)
            ValidFields |= PPDELTAUPDATED_TOR_X;

        if(Torque.Y != other->Torque.Y)
            ValidFields |= PPDELTAUPDATED_TOR_Y;

        if(Torque.Z != other->Torque.Z)
            ValidFields |= PPDELTAUPDATED_TOR_Z;
        
    }

    packet << ValidFields;

    // Add the changed data to the packet //
    // Position
    if(ValidFields & PPDELTAUPDATED_POS_X)
        packet << Position.X;

    if(ValidFields & PPDELTAUPDATED_POS_Y)
        packet << Position.Y;
    
    if(ValidFields & PPDELTAUPDATED_POS_Z)
        packet << Position.Z;

    // Rotation
    if(ValidFields & PPDELTAUPDATED_ROT_X)
        packet << Rotation.X;

    if(ValidFields & PPDELTAUPDATED_ROT_Y)
        packet << Rotation.Y;
    
    if(ValidFields & PPDELTAUPDATED_ROT_Z)
        packet << Rotation.Z;

    if(ValidFields & PPDELTAUPDATED_ROT_W)
        packet << Rotation.W;

    // Velocity
    if(ValidFields & PPDELTAUPDATED_VEL_X)
        packet << Velocity.X;

    if(ValidFields & PPDELTAUPDATED_VEL_Y)
        packet << Velocity.Y;
    
    if(ValidFields & PPDELTAUPDATED_VEL_Z)
        packet << Velocity.Z;

    // Torque
    if(ValidFields & PPDELTAUPDATED_TOR_X)
        packet << Torque.X;

    if(ValidFields & PPDELTAUPDATED_TOR_Y)
        packet << Torque.Y;
    
    if(ValidFields & PPDELTAUPDATED_TOR_Z)
        packet << Torque.Z;
    
}
// ------------------------------------ //
DLLEXPORT bool PhysicalDeltaState::FillMissingData(ObjectDeltaStateData &otherstate){

    const PhysicalDeltaState &other = static_cast<PhysicalDeltaState&>(
        otherstate);

    if(ValidFields == 0){

        // Copy everything as nothing is valid //
        Position = other.Position;
        Rotation = other.Rotation;
        Velocity = other.Velocity;
        Torque = other.Torque;

        ValidFields = other.ValidFields;
        return other.ValidFields == PPDELTA_ALL_UPDATED;
        
    } else if(ValidFields == PPDELTA_ALL_UPDATED){
        
        // Already contains everything //
        return true;
    }

    bool allsucceeded = true;

    // Position
    if(!(ValidFields & PPDELTAUPDATED_POS_X)){
        if(other.ValidFields & PPDELTAUPDATED_POS_X){
            
            Position.X = other.Position.X;
        } else {

            allsucceeded = false;
        }
    }

    if(!(ValidFields & PPDELTAUPDATED_POS_Y)){
        if(other.ValidFields & PPDELTAUPDATED_POS_Y){
            
            Position.Y = other.Position.Y;
        } else {

            allsucceeded = false;
        }
    }
    
    if(!(ValidFields & PPDELTAUPDATED_POS_Z)){
        if(other.ValidFields & PPDELTAUPDATED_POS_Z){
            
            Position.Z = other.Position.Z;
        } else {

            allsucceeded = false;
        }
    }

    // Rotation
    if(!(ValidFields & PPDELTAUPDATED_ROT_X)){
        if(other.ValidFields & PPDELTAUPDATED_ROT_X){
            
            Rotation.X = other.Rotation.X;
        } else {

            allsucceeded = false;
        }
    }

    if(!(ValidFields & PPDELTAUPDATED_ROT_Y)){
        if(other.ValidFields & PPDELTAUPDATED_ROT_Y){
            
            Rotation.Y = other.Rotation.Y;
        } else {

            allsucceeded = false;
        }
    }
    
    if(!(ValidFields & PPDELTAUPDATED_ROT_Z)){
        if(other.ValidFields & PPDELTAUPDATED_ROT_Z){
            
            Rotation.Z = other.Rotation.Z;
        } else {

            allsucceeded = false;
        }
    }

    if(!(ValidFields & PPDELTAUPDATED_ROT_W)){
        if(other.ValidFields & PPDELTAUPDATED_ROT_W){
            
            Rotation.W = other.Rotation.W;
        } else {

            allsucceeded = false;
        }
    }

    // Velocity
    if(!(ValidFields & PPDELTAUPDATED_VEL_X)){
        if(other.ValidFields & PPDELTAUPDATED_VEL_X){
            
            Position.X = other.Position.X;
        } else {

            allsucceeded = false;
        }
    }

    if(!(ValidFields & PPDELTAUPDATED_VEL_Y)){
        if(other.ValidFields & PPDELTAUPDATED_VEL_Y){
            
            Position.Y = other.Position.Y;
        } else {

            allsucceeded = false;
        }
    }
    
    if(!(ValidFields & PPDELTAUPDATED_VEL_Z)){
        if(other.ValidFields & PPDELTAUPDATED_VEL_Z){
            
            Position.Z = other.Position.Z;
        } else {

            allsucceeded = false;
        }
    }

    // Torque
    if(!(ValidFields & PPDELTAUPDATED_TOR_X)){
        if(other.ValidFields & PPDELTAUPDATED_TOR_X){
            
            Torque.X = other.Torque.X;
        } else {

            allsucceeded = false;
        }
    }

    if(!(ValidFields & PPDELTAUPDATED_TOR_Y)){
        if(other.ValidFields & PPDELTAUPDATED_TOR_Y){
            
            Torque.Y = other.Torque.Y;
        } else {

            allsucceeded = false;
        }
    }
    
    if(!(ValidFields & PPDELTAUPDATED_TOR_Z)){
        if(other.ValidFields & PPDELTAUPDATED_TOR_Z){
            
            Torque.Z = other.Torque.Z;
        } else {

            allsucceeded = false;
        }
    }

    ValidFields |= other.ValidFields;
    return allsucceeded;
}
// ------------------ PositionDeltaState ------------------ //
DLLEXPORT PositionDeltaState::PositionDeltaState(int tick,
    const Float3 &position, const Float4 &rotation) :
    ObjectDeltaStateData(tick), Position(position), Rotation(rotation), ValidFields(PRDELTA_ALL_UPDATED)
{

}

DLLEXPORT PositionDeltaState::PositionDeltaState(int tick,
    sf::Packet &packet) :
    ObjectDeltaStateData(tick)
{
    packet >> ValidFields;

    if(!packet)
        throw InvalidArgument("invalid packet for positionable rotationable delta state");
    
    // Position
    if(ValidFields & PRDELTAUPDATED_POS_X)
        packet >> Position.X;

    if(ValidFields & PRDELTAUPDATED_POS_Y)
        packet >> Position.Y;
    
    if(ValidFields & PRDELTAUPDATED_POS_Z)
        packet >> Position.Z;

    // Rotation
    if(ValidFields & PRDELTAUPDATED_ROT_X)
        packet >> Rotation.X;

    if(ValidFields & PRDELTAUPDATED_ROT_Y)
        packet >> Rotation.Y;
    
    if(ValidFields & PRDELTAUPDATED_ROT_Z)
        packet >> Rotation.Z;
    
    if(ValidFields & PRDELTAUPDATED_ROT_W)
        packet >> Rotation.W;

    if(!packet)
        throw InvalidArgument("invalid packet for positionable rotationable delta state");
}

DLLEXPORT PositionDeltaState::~PositionDeltaState(){

}
// ------------------------------------ //
DLLEXPORT void PositionDeltaState::CreateUpdatePacket(ObjectDeltaStateData* olderstate,
    sf::Packet &packet)
{
    ValidFields = 0;
    
    // Check which parts have changed //
    if(!olderstate){

        // When comparing against NULL state everything is updated //
        ValidFields = PRDELTA_ALL_UPDATED;
        
    } else {

        auto other = static_cast<PositionDeltaState*>(olderstate);
        
        // Position
        if(Position.X != other->Position.X)
            ValidFields |= PRDELTAUPDATED_POS_X;

        if(Position.Y != other->Position.Y)
            ValidFields |= PRDELTAUPDATED_POS_Y;

        if(Position.Z != other->Position.Z)
            ValidFields |= PRDELTAUPDATED_POS_Z;

        // Rotation
        if(Rotation.X != other->Rotation.X)
            ValidFields |= PRDELTAUPDATED_ROT_X;

        if(Rotation.Y != other->Rotation.Y)
            ValidFields |= PRDELTAUPDATED_ROT_Y;

        if(Rotation.Z != other->Rotation.Z)
            ValidFields |= PRDELTAUPDATED_ROT_Z;

        if(Rotation.W != other->Rotation.W)
            ValidFields |= PRDELTAUPDATED_ROT_W;
    }

    packet << ValidFields;

    // Add the changed data to the packet //
    // Position
    if(ValidFields & PRDELTAUPDATED_POS_X)
        packet << Position.X;

    if(ValidFields & PRDELTAUPDATED_POS_Y)
        packet << Position.Y;
    
    if(ValidFields & PRDELTAUPDATED_POS_Z)
        packet << Position.Z;

    // Rotation
    if(ValidFields & PRDELTAUPDATED_ROT_X)
        packet << Rotation.X;

    if(ValidFields & PRDELTAUPDATED_ROT_Y)
        packet << Rotation.Y;
    
    if(ValidFields & PRDELTAUPDATED_ROT_Z)
        packet << Rotation.Z;

    if(ValidFields & PRDELTAUPDATED_ROT_W)
        packet << Rotation.W;
}
        
DLLEXPORT bool PositionDeltaState::FillMissingData(ObjectDeltaStateData &otherstate){

    const PositionDeltaState &other =
        static_cast<PositionDeltaState&>(otherstate);

    if(ValidFields == 0){

        // Copy everything as nothing is valid //
        Position = other.Position;
        Rotation = other.Rotation;

        ValidFields = other.ValidFields;
        return other.ValidFields == PRDELTA_ALL_UPDATED;
        
    } else if(ValidFields == PRDELTA_ALL_UPDATED){
        
        // Already contains everything //
        return true;
    }

    bool allsucceeded = true;

    // Position
    if(!(ValidFields & PRDELTAUPDATED_POS_X)){
        if(other.ValidFields & PRDELTAUPDATED_POS_X){
            
            Position.X = other.Position.X;
        } else {

            allsucceeded = false;
        }
    }

    if(!(ValidFields & PRDELTAUPDATED_POS_Y)){
        if(other.ValidFields & PRDELTAUPDATED_POS_Y){
            
            Position.Y = other.Position.Y;
        } else {

            allsucceeded = false;
        }
    }
    
    if(!(ValidFields & PRDELTAUPDATED_POS_Z)){
        if(other.ValidFields & PRDELTAUPDATED_POS_Z){
            
            Position.Z = other.Position.Z;
        } else {

            allsucceeded = false;
        }
    }

    // Rotation
    if(!(ValidFields & PRDELTAUPDATED_ROT_X)){
        if(other.ValidFields & PRDELTAUPDATED_ROT_X){
            
            Rotation.X = other.Rotation.X;
        } else {

            allsucceeded = false;
        }
    }

    if(!(ValidFields & PRDELTAUPDATED_ROT_Y)){
        if(other.ValidFields & PRDELTAUPDATED_ROT_Y){
            
            Rotation.Y = other.Rotation.Y;
        } else {

            allsucceeded = false;
        }
    }
    
    if(!(ValidFields & PRDELTAUPDATED_ROT_Z)){
        if(other.ValidFields & PRDELTAUPDATED_ROT_Z){
            
            Rotation.Z = other.Rotation.Z;
        } else {

            allsucceeded = false;
        }
    }

    if(!(ValidFields & PRDELTAUPDATED_ROT_W)){
        if(other.ValidFields & PRDELTAUPDATED_ROT_W){
            
            Rotation.W = other.Rotation.W;
        } else {

            allsucceeded = false;
        }
    }

    ValidFields |= other.ValidFields;
    
    return allsucceeded;
}
// ------------------ TrackControllerState ------------------ //
DLLEXPORT TrackControllerState::TrackControllerState(int tick, int reached,
    float speed, float progress) :
    ObjectDeltaStateData(tick), ReachedNode(reached), ChangeSpeed(speed),
    NodeProgress(progress), ValidFields(TRACKSTATE_UPDATED_ALL), AddedNodes(0)
{

}

DLLEXPORT TrackControllerState::TrackControllerState(int tick,
    sf::Packet &packet) :
    ObjectDeltaStateData(tick)
{

    // We need to do the opposite of what we do in CreateUpdatePacket //
    packet >> ValidFields;

    if(!packet)
        throw InvalidArgument("invalid packet for TrackControllerState");
    
    if(ValidFields & TRACKSTATE_UPDATED_NODE)
        packet >> ReachedNode;

    if(ValidFields & TRACKSTATE_UPDATED_SPEED)
        packet >> ChangeSpeed;
    
    if(ValidFields & TRACKSTATE_UPDATED_PROGRESS)
        packet >> NodeProgress;

    if(!packet)
        throw InvalidArgument("invalid packet for TrackControllerState");
}
            
DLLEXPORT void TrackControllerState::CreateUpdatePacket(ObjectDeltaStateData* olderstate,
    sf::Packet &packet)
{

    ValidFields = 0;

    // Check which parts have changed //
    if(!olderstate){

        // When comparing against NULL state everything is updated //
        ValidFields = TRACKSTATE_UPDATED_ALL;
        
    } else {

        TrackControllerState* other = static_cast<TrackControllerState*>(olderstate);

        // Node
        if(ReachedNode != other->ReachedNode)
            ValidFields |= TRACKSTATE_UPDATED_NODE;

        // Speed
        if(ChangeSpeed != other->ChangeSpeed)
            ValidFields |= TRACKSTATE_UPDATED_SPEED;

        // Progress
        if(NodeProgress != other->NodeProgress)
            ValidFields |= TRACKSTATE_UPDATED_PROGRESS;
    }

    packet << ValidFields;

    // Add the changed data to the packet //
    if(ValidFields & TRACKSTATE_UPDATED_NODE)
        packet << ReachedNode;

    if(ValidFields & TRACKSTATE_UPDATED_SPEED)
        packet << ChangeSpeed;
    
    if(ValidFields & TRACKSTATE_UPDATED_PROGRESS)
        packet << NodeProgress;
}
// ------------------------------------ //
DLLEXPORT bool TrackControllerState::FillMissingData(ObjectDeltaStateData &otherstate){
    
    const TrackControllerState &other = static_cast<TrackControllerState&>(otherstate);

    if(ValidFields == 0){
        
        // Copy everything as nothing is valid //
        ReachedNode = other.ReachedNode;
        ChangeSpeed = other.ChangeSpeed;
        NodeProgress = other.NodeProgress;

        
        return other.ValidFields == TRACKSTATE_UPDATED_ALL;
        
    } else if(ValidFields == TRACKSTATE_UPDATED_ALL){
        
        // Already contains everything //
        return true;
    }

    bool allsucceeded = true;

    // ReachedNode
    if(!(ValidFields & TRACKSTATE_UPDATED_NODE)){
        if(other.ValidFields & TRACKSTATE_UPDATED_NODE){
            
            ReachedNode = other.ReachedNode;
        } else {

            allsucceeded = false;
        }
    }

    // ChangeSpeed
    if(!(ValidFields & TRACKSTATE_UPDATED_SPEED)){
        if(other.ValidFields & TRACKSTATE_UPDATED_SPEED){
            
            ChangeSpeed = other.ChangeSpeed;
        } else {

            allsucceeded = false;
        }
    }

    // NodeProgress
    if(!(ValidFields & TRACKSTATE_UPDATED_PROGRESS)){
        if(other.ValidFields & TRACKSTATE_UPDATED_PROGRESS){
            
            NodeProgress = other.NodeProgress;
        } else {

            allsucceeded = false;
        }
    }

    ValidFields |= other.ValidFields;

    return allsucceeded;
}


