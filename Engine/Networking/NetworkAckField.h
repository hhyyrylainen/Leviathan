// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

#include <map>
#include <vector>
#include <functional>
#include <memory>

namespace sf{
class Packet;
}


namespace Leviathan{

//! \brief For keeping track of received remote packets.
//!
//! These are used to populate ack fields for outgoing packets
enum class RECEIVED_STATE{

    //! Packet hasn't been received
    NotReceived = 0,

    //! Packet is received but no acks have been sent
    StateReceived,

    //! Packet is received and an ack has been sent
    //! This is basically the same as ReceivedAckSucceeded
    AcksSent,

    //! Packet is received and the ack is also received
    //! At this point we no longer need to think about this packet
    ReceivedAckSucceeded
};

class NetworkAckField{
public:

    using PacketReceiveStatus = std::map<uint32_t, RECEIVED_STATE>;

    //! \brief Copies acts from copyfrom starting with the number firstpacketid
    //!
    //! Marks them as RECEIVED_STATE::AcksSent
    DLLEXPORT NetworkAckField(uint32_t firstpacketid, uint8_t maxacks,
        PacketReceiveStatus &copyfrom);

    DLLEXPORT NetworkAckField(sf::Packet &packet);

    
    DLLEXPORT void AddDataToPacket(sf::Packet &packet) const;

    //! \returns True if an ack number FirstPacketID + ackindex is set
    inline bool IsAckSet(uint8_t ackindex) const{
        
        // We can use division to find out which vector element is wanted //
        size_t vecelement = ackindex / 8;

        if (vecelement >= Acks.size())
            return false;

        return (Acks[vecelement] & (1 << (ackindex % 8))) != 0;
    }

    //! \brief Calls func with each set ack
    DLLEXPORT void InvokeForEachAck(std::function<void (uint32_t)> func) const;

    // Data //
    uint32_t FirstPacketID;
    std::vector<uint8_t> Acks;
};


//! \brief Holds sent ack packets in order to mark the acks as properly sent
struct SentAcks{

    SentAcks(uint32_t localpacketid, std::shared_ptr<NetworkAckField> acks) :
        InsidePacket(localpacketid), AcksInThePacket(acks)
    {}

    //! The packet (SentNetworkThing) in which these acks were sent //
    uint32_t InsidePacket;

    std::shared_ptr<NetworkAckField> AcksInThePacket;
        
    //! Marks when the remote host tells us that any packet in which
    //! bunch is in is received
    bool Received = false;
};

}
