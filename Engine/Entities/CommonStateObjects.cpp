// ------------------------------------ //
#ifndef LEVIATHAN_COMMONSTATEOBJECTS
#include "CommonStateObjects.h"
#endif
#include "Exceptions/ExceptionInvalidArgument.h"
using namespace Leviathan;
// ------------------------------------ //

// ------------------ PositionablePhysicalDeltaState ------------------ //
DLLEXPORT Leviathan::PositionablePhysicalDeltaState::PositionablePhysicalDeltaState(const Float3 &position,
    const Float3 &velocity, const Float3 &torque) :
    Position(position), Velocity(velocity), Torque(torque)
{

}

DLLEXPORT Leviathan::PositionablePhysicalDeltaState::PositionablePhysicalDeltaState(sf::Packet &packet,
    shared_ptr<ObjectDeltaStateData> fillblanks)
{
    int16_t packetstates = 0;

    // We need to do the opposite of what we do in CreateUpdatePacket //
    packet >> packetstates;

    if(!packet)
        throw ExceptionInvalidArgument(L"invalid packet for positionable delta state", 0, __WFUNCTION__,
            L"packet", L"");
    
    // Warn if we can't fill in the blanks //
    if(!fillblanks){

        if(packetstates != PPDELTA_ALL_UPDATED){

            // TODO: add a mechanism for automatically reporting engine bugs that shouldn't happen //
            Logger::Get()->Error("PositionablePhysicalDeltaState: trying to reconstruct packet update without older "
                "state, PLEASE REPORT THIS ERROR");
            
            Position = Float3(0);
            Velocity = Float3(0);
            Torque = Float3(0);
        }
        
    } else {

        // Take starting values from the fillblanks one //
        (*this) = *static_cast<PositionablePhysicalDeltaState*>(fillblanks.get());
    }

    // Position
    if(packetstates & PPDELTAUPDATED_POS_X)
        packet >> Position.X;

    if(packetstates & PPDELTAUPDATED_POS_Y)
        packet >> Position.Y;
    
    if(packetstates & PPDELTAUPDATED_POS_Z)
        packet >> Position.Z;

    // Velocity
    if(packetstates & PPDELTAUPDATED_VEL_X)
        packet >> Velocity.X;

    if(packetstates & PPDELTAUPDATED_VEL_Y)
        packet >> Velocity.Y;
    
    if(packetstates & PPDELTAUPDATED_VEL_Z)
        packet >> Velocity.Z;

    // Torque
    if(packetstates & PPDELTAUPDATED_TOR_X)
        packet >> Torque.X;

    if(packetstates & PPDELTAUPDATED_TOR_Y)
        packet >> Torque.Y;
    
    if(packetstates & PPDELTAUPDATED_TOR_Z)
        packet >> Torque.Z;
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
        changedparts = PPDELTA_ALL_UPDATED;
        
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

    // Velocity
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
