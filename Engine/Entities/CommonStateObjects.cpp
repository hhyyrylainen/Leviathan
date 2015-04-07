// ------------------------------------ //
#ifndef LEVIATHAN_COMMONSTATEOBJECTS
#include "CommonStateObjects.h"
#endif
#include "Exceptions.h"
using namespace Leviathan;
// ------------------------------------ //

// ------------------ PositionablePhysicalDeltaState ------------------ //
DLLEXPORT Leviathan::PositionablePhysicalDeltaState::PositionablePhysicalDeltaState(int tick, const Float3 &position,
    const Float4 &rotation, const Float3 &velocity, const Float3 &torque) :
    ObjectDeltaStateData(tick), Position(position), Velocity(velocity), Torque(torque), Rotation(rotation),
    ValidFields(PPDELTA_ALL_UPDATED)
{

}

DLLEXPORT Leviathan::PositionablePhysicalDeltaState::PositionablePhysicalDeltaState(int tick,
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

DLLEXPORT Leviathan::PositionablePhysicalDeltaState::~PositionablePhysicalDeltaState(){


}
// ------------------------------------ //
DLLEXPORT void Leviathan::PositionablePhysicalDeltaState::CreateUpdatePacket(ObjectDeltaStateData* olderstate,
    sf::Packet &packet)
{

    ValidFields = 0;

    // Check which parts have changed //
    if(!olderstate){

        // When comparing against NULL state everything is updated //
        ValidFields = PPDELTA_ALL_UPDATED;
        
    } else {

        PositionablePhysicalDeltaState* other = static_cast<PositionablePhysicalDeltaState*>(olderstate);

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
DLLEXPORT bool PositionablePhysicalDeltaState::FillMissingData(ObjectDeltaStateData &otherstate){

    const PositionablePhysicalDeltaState &other = static_cast<PositionablePhysicalDeltaState&>(
        otherstate);

    if(ValidFields == 0){

        // Copy everything as nothing is valid //
        Position = other.Position;
        Rotation = other.Rotation;
        Velocity = other.Velocity;
        Torque = other.Torque;

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
    
    return allsucceeded;
}
// ------------------ PositionableRotationableDeltaState ------------------ //
DLLEXPORT PositionableRotationableDeltaState::PositionableRotationableDeltaState(int tick,
    const Float3 &position, const Float4 &rotation) :
    ObjectDeltaStateData(tick), Position(position), Rotation(rotation), ValidFields(PRDELTA_ALL_UPDATED)
{

}

DLLEXPORT PositionableRotationableDeltaState::PositionableRotationableDeltaState(int tick,
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

DLLEXPORT PositionableRotationableDeltaState::~PositionableRotationableDeltaState(){

}
// ------------------------------------ //
DLLEXPORT void PositionableRotationableDeltaState::CreateUpdatePacket(ObjectDeltaStateData* olderstate,
    sf::Packet &packet)
{
    ValidFields = 0;
    
    // Check which parts have changed //
    if(!olderstate){

        // When comparing against NULL state everything is updated //
        ValidFields = PRDELTA_ALL_UPDATED;
        
    } else {

        auto other = static_cast<PositionableRotationableDeltaState*>(olderstate);
        
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
        
DLLEXPORT bool PositionableRotationableDeltaState::FillMissingData(ObjectDeltaStateData &otherstate){

    const PositionableRotationableDeltaState &other =
        static_cast<PositionableRotationableDeltaState&>(otherstate);

    if(ValidFields == 0){

        // Copy everything as nothing is valid //
        Position = other.Position;
        Rotation = other.Rotation;

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
    
    return allsucceeded;
}


