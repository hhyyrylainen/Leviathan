// ------------------------------------ //
#include "NetworkAckField.h"

#include "SFML/Network/Packet.hpp"

using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Leviathan::NetworkAckField::NetworkAckField(
    uint32_t firstpacketid, uint8_t maxacks, PacketReceiveStatus& copyfrom) :
    FirstPacketID(firstpacketid)
{
    // Id is 0 nothing should be copied //
    if(FirstPacketID == 0)
        return;

    // Before we reach the first packet id we will want to skip stuff //
    bool foundstart = false;

    for(auto iter = copyfrom.begin(); iter != copyfrom.end(); ++iter) {

        // Skip current if not received //
        if(iter->second == RECEIVED_STATE::NotReceived)
            continue;

        if(!foundstart) {

            if(iter->first >= FirstPacketID) {


                // Adjust the starting index, if it is an exact match for firstpacketid
                // this should be 0
                auto startindex = iter->first - FirstPacketID;

                // Check that the index is within the size of the field, otherwise fail
                if(startindex >= maxacks)
                    break;

                foundstart = true;

            } else {

                continue;
            }
        }

        size_t currentindex = iter->first - FirstPacketID;

        if(currentindex >= maxacks)
            break;

        // Copy current ack //
        const auto vecelement = currentindex / 8;

        while(Acks.size() <= vecelement) {

            Acks.push_back(0);
        }

        Acks[vecelement] |= (1 << (currentindex % 8));

        // Stop copying once enough acks have been set. The loop can end before this, but
        // that just leaves the rest of the acks as 0
        if(currentindex + 1 >= maxacks)
            break;
    }

    if(!foundstart) {

        // No acks to send //
        FirstPacketID = 0;
        return;
    }
}

DLLEXPORT NetworkAckField::NetworkAckField(sf::Packet& packet)
{
    // Get data //
    packet >> FirstPacketID;

    // Empty ack fields //
    if(FirstPacketID == 0)
        return;

    uint8_t tmpsize = 0;

    packet >> tmpsize;

    if(!packet)
        return;

    // Fill in the acks from the packet //
    Acks.resize(tmpsize);

    for(char i = 0; i < tmpsize; i++) {
        packet >> Acks[i];
    }
}
// ------------------------------------ //
DLLEXPORT void NetworkAckField::AddDataToPacket(sf::Packet& packet) const
{
    packet << FirstPacketID;

    if(FirstPacketID == 0)
        return;

    uint8_t tmpsize = static_cast<uint8_t>(Acks.size());
    packet << tmpsize;

    // fill in the ack data //
    for(uint8_t i = 0; i < tmpsize; i++) {

        packet << Acks[i];
    }
}

DLLEXPORT void NetworkAckField::InvokeForEachAck(std::function<void(uint32_t)> func) const
{
    const auto size = static_cast<uint32_t>(Acks.size());
    for(uint32_t i = 0; i < size; ++i) {

        for(uint8_t bit = 0; bit < 8; ++bit) {

            const auto id = (i * 8) + bit + FirstPacketID;

            if(Acks[i] & (1 << bit))
                func(id);
        }
    }
}
