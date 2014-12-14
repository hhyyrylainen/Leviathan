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

    int16_t changedparts;

    // Check which parts have changed //


    packet << changedparts;

    // Add the changed data to the packet //

    
}
