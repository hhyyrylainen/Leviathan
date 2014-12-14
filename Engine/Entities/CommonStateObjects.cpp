// ------------------------------------ //
#ifndef LEVIATHAN_COMMONSTATEOBJECTS
#include "CommonStateObjects.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

// ------------------ PositionablePhysicalDeltaState ------------------ //
DLLEXPORT Leviathan::PositionablePhysicalDeltaState::PositionablePhysicalDeltaState(const Float3 &position,
    const Float3 &velocity, const Float3 &torque) :
    Position(position), Velocity(velocity), Torque(torque)
{

}

DLLEXPORT Leviathan::PositionablePhysicalDeltaState::~PositionablePhysicalDeltaState(){


}
// ------------------------------------ //
DLLEXPORT void Leviathan::PositionablePhysicalDeltaState::CreateUpdatePacket(ObjectDeltaStateData* olderstate,
    sf::Packet &packet)
{

    int16_t changedparts = 0;

    // Check which parts have changed //
    if(!olderstate){

        // When comparing against NULL state everything is updated //
        // Set to max 16 bit value to set all to one //
        changedparts = 65535;
        
    } else {

        PositionablePhysicalDeltaState* other = static_cast<PositionablePhysicalDeltaState*>(olderstate);

        // Position
        if(Position.X != other->Position.X)
            changedparts |= PPDELTAUPDATED_POS_X;

        if(Position.Y != other->Position.Y)
            changedparts |= PPDELTAUPDATED_POS_Y;

        if(Position.Z != other->Position.Z)
            changedparts |= PPDELTAUPDATED_POS_Z;

        // Velocity
        if(Velocity.X != other->Velocity.X)
            changedparts |= PPDELTAUPDATED_VEL_X;

        if(Velocity.Y != other->Velocity.Y)
            changedparts |= PPDELTAUPDATED_VEL_Y;

        if(Velocity.Z != other->Velocity.Z)
            changedparts |= PPDELTAUPDATED_VEL_Z;


        // Torque
        if(Torque.X != other->Torque.X)
            changedparts |= PPDELTAUPDATED_TOR_X;

        if(Torque.Y != other->Torque.Y)
            changedparts |= PPDELTAUPDATED_TOR_Y;

        if(Torque.Z != other->Torque.Z)
            changedparts |= PPDELTAUPDATED_TOR_Z;
        
    }

    packet << changedparts;

    // Add the changed data to the packet //
    if(changedparts & PPDELTAUPDATED_POS_X)
        packet << Position.X;

    if(changedparts & PPDELTAUPDATED_POS_Y)
        packet << Position.Y;
    
    if(changedparts & PPDELTAUPDATED_POS_Z)
        packet << Position.Z;

    // Velcity
    if(changedparts & PPDELTAUPDATED_VEL_X)
        packet << Velocity.X;

    if(changedparts & PPDELTAUPDATED_VEL_Y)
        packet << Velocity.Y;
    
    if(changedparts & PPDELTAUPDATED_VEL_Z)
        packet << Velocity.Z;

    // Torque
    if(changedparts & PPDELTAUPDATED_TOR_X)
        packet << Torque.X;

    if(changedparts & PPDELTAUPDATED_TOR_Y)
        packet << Torque.Y;
    
    if(changedparts & PPDELTAUPDATED_TOR_Z)
        packet << Torque.Z;
    
}
