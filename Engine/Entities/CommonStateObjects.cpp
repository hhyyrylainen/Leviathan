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
    ObjectDeltaStateData(tick), Position(position), Velocity(velocity), Torque(torque), Rotation(rotation)
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
// ------------------ PositionableRotationableDeltaState ------------------ //
