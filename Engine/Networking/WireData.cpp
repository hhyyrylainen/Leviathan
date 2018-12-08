// ------------------------------------ //
#include "WireData.h"

#include "NetworkAckField.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"

#include "SentNetworkThing.h"


#include "SFML/Network/Packet.hpp"

#include <array>
#include <limits>

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT std::shared_ptr<SentRequest> WireData::FormatRequestBytes(
    const std::shared_ptr<NetworkRequest>& request, RECEIVE_GUARANTEE guarantee,
    uint32_t messagenumber, uint32_t localpacketid, const NetworkAckField* acks,
    sf::Packet& bytesreceiver)
{
    LEVIATHAN_ASSERT(request, "trying to generate packet data for empty request");

    bytesreceiver.clear();

    std::array<uint32_t, 1> messages;
    // Requests don't use this marking mechanism
    messages[0] = messagenumber;

    // We need a complete header with acks and stuff //
    PrepareHeaderForPacket(localpacketid, &messages[0], 1, acks, bytesreceiver);

    // Request type //
    bytesreceiver << NORMAL_REQUEST_TYPE;

    // Message number //
    bytesreceiver << messagenumber;

    // Pack the message data in //
    request->AddDataToPacket(bytesreceiver);

    return std::make_shared<SentRequest>(localpacketid, messagenumber, guarantee, request);
}

DLLEXPORT void WireData::FormatRequestBytes(const NetworkRequest& request,
    uint32_t messagenumber, uint32_t localpacketid, const NetworkAckField* acks,
    sf::Packet& bytesreceiver)
{
    bytesreceiver.clear();

    std::array<uint32_t, 1> messages;
    // Requests don't use this marking mechanism
    messages[0] = messagenumber;

    // We need a complete header with acks and stuff //
    PrepareHeaderForPacket(localpacketid, &messages[0], 1, acks, bytesreceiver);

    // Request type //
    bytesreceiver << NORMAL_REQUEST_TYPE;

    // Message number //
    bytesreceiver << messagenumber;

    // Pack the message data in //
    request.AddDataToPacket(bytesreceiver);
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<SentResponse> WireData::FormatResponseBytes(
    const std::shared_ptr<NetworkResponse>& response, RECEIVE_GUARANTEE guarantee,
    uint32_t messagenumber, uint32_t localpacketid, const NetworkAckField* acks,
    sf::Packet& bytesreceiver)
{
    LEVIATHAN_ASSERT(response, "trying to generate packet data for empty response");

    bytesreceiver.clear();

    std::array<uint32_t, 1> messages;
    // Requests don't use this marking mechanism
    messages[0] = messagenumber;

    // We need a complete header with acks and stuff //
    PrepareHeaderForPacket(localpacketid, &messages[0], 1, acks, bytesreceiver);

    // Request type //
    bytesreceiver << NORMAL_RESPONSE_TYPE;

    // Message number //
    bytesreceiver << messagenumber;

    // Pack the message data in //
    response->AddDataToPacket(bytesreceiver);

    return std::make_shared<SentResponse>(localpacketid, messagenumber, guarantee, response);
}

DLLEXPORT void WireData::FormatResponseBytes(const NetworkResponse& response,
    uint32_t messagenumber, uint32_t localpacketid, const NetworkAckField* acks,
    sf::Packet& bytesreceiver)
{
    bytesreceiver.clear();

    std::array<uint32_t, 1> messages;
    // Requests don't use this marking mechanism
    messages[0] = messagenumber;

    // We need a complete header with acks and stuff //
    PrepareHeaderForPacket(localpacketid, &messages[0], 1, acks, bytesreceiver);

    // Request type //
    bytesreceiver << NORMAL_RESPONSE_TYPE;

    // Message number //
    bytesreceiver << messagenumber;

    // Pack the message data in //
    response.AddDataToPacket(bytesreceiver);
}

DLLEXPORT std::shared_ptr<SentResponse> WireData::FormatResponseBytesTracked(
    const NetworkResponse& response, uint32_t messagenumber, uint32_t localpacketid,
    const NetworkAckField* acks, sf::Packet& bytesreceiver)
{
    bytesreceiver.clear();

    std::array<uint32_t, 1> messages;
    // Requests don't use this marking mechanism
    messages[0] = messagenumber;

    // We need a complete header with acks and stuff //
    PrepareHeaderForPacket(localpacketid, &messages[0], 1, acks, bytesreceiver);

    // Request type //
    bytesreceiver << NORMAL_RESPONSE_TYPE;

    // Message number //
    bytesreceiver << messagenumber;

    // Pack the message data in //
    response.AddDataToPacket(bytesreceiver);

    return std::make_shared<SentResponse>(localpacketid, messagenumber, response);
}
// ------------------------------------ //
DLLEXPORT void WireData::FormatAckOnlyPacket(
    const std::vector<uint32_t>& packetstoack, sf::Packet& bytesreceiver)
{
    bytesreceiver << LEVIATHAN_ACK_PACKET;

    LEVIATHAN_ASSERT(packetstoack.size() < std::numeric_limits<uint8_t>::max(),
        "CreateAckOnlyPacket too many packetstoack provided, can't fit in uint8");

    const uint8_t count = static_cast<uint8_t>(packetstoack.size());

    bytesreceiver << count;

    for(uint8_t i = 0; i < count; ++i) {

        bytesreceiver << packetstoack[i];
    }
}
// ------------------------------------ //
DLLEXPORT void WireData::DecodeIncomingData(sf::Packet& packet,
    const std::function<DECODE_CALLBACK_RESULT(NetworkAckField&)>& ackcallback,
    const std::function<void(uint32_t)>& singleack,
    const std::function<DECODE_CALLBACK_RESULT(uint32_t)>& packetnumberreceived,
    const std::function<DECODE_CALLBACK_RESULT(uint8_t, uint32_t, sf::Packet&)>&
        messagereceived)
{
    // Header //
    uint16_t leviathanMagic = 0;
    packet >> leviathanMagic;

    if(!packet) {

        Logger::Get()->Error("Received packet has invalid (header) format");
        return;
    }

    switch(leviathanMagic) {
    case LEVIATHAN_NORMAL_PACKET: {
        uint32_t packetNumber = 0;
        packet >> packetNumber;

        if(!packet) {

            Logger::Get()->Error("Received packet has invalid (packet number) format");
            return;
        }

        NetworkAckField otherreceivedpackages(packet);

        if(!packet) {

            Logger::Get()->Error("Received packet has invalid format");
        }

        // Messages //
        uint8_t messageCount = 0;

        if(!(packet >> messageCount)) {

            Logger::Get()->Error("Received packet has invalid format, missing Message Count");
        }

        // Marks things as successfully sent //
        if(ackcallback) {

            auto callbackResult = ackcallback(otherreceivedpackages);

            if(callbackResult != DECODE_CALLBACK_RESULT::Continue) {

                LOG_ERROR("Packet decode callback signaled error");
                return;
            }
        }

        // Report the packet as received //
        if(packetnumberreceived) {

            auto callbackResult = packetnumberreceived(packetNumber);

            if(callbackResult != DECODE_CALLBACK_RESULT::Continue) {

                LOG_ERROR("Packet decode callback signaled error");
                return;
            }
        }

        for(int i = 0; i < messageCount; ++i) {

            uint8_t messageType = 0;
            packet >> messageType;

            uint32_t messageNumber = 0;
            packet >> messageNumber;

            if(!packet) {

                LOG_ERROR("Connection: received packet has an invalid message "
                          "(some may have been processed already)");
                return;
            }

            auto callbackResult = messagereceived(messageType, messageNumber, packet);

            if(callbackResult != DECODE_CALLBACK_RESULT::Continue) {

                LOG_ERROR("Packet decode callback signaled error");
                return;
            }
        }

        return;
    }
    case LEVIATHAN_ACK_PACKET: {
        uint8_t ackCount = 0;

        if(!(packet >> ackCount)) {

            LOG_ERROR("Received packet has invalid (ack number) format");
            return;
        }

        for(uint8_t i = 0; i < ackCount; ++i) {

            uint32_t ack = 0;

            if(!(packet >> ack)) {

                LOG_ERROR("Received packet ended while acks were being unpacked, "
                          "some were applied");
                return;
            }

            if(singleack)
                singleack(ack);
        }

        return;
    }
    default: {
        LOG_ERROR("Received packet has an unknown type: " + Convert::ToString(leviathanMagic));
        return;
    }
    }
}

// ------------------------------------ //
DLLEXPORT void WireData::PrepareHeaderForPacket(uint32_t localpacketid,
    uint32_t* firstmessagenumber, size_t messagenumbercount,
    const Leviathan::NetworkAckField* acks, sf::Packet& tofill)
{
    LEVIATHAN_ASSERT(localpacketid > 0, "Trying to fill packet with packetid == 0");

    // See the doxygen page networkformat for the header format //

    // Type //
    tofill << LEVIATHAN_NORMAL_PACKET;

    // PKT ID //
    tofill << localpacketid;

    // Acks
    FillHeaderAckData(acks, tofill);


    // Message count
    LEVIATHAN_ASSERT(messagenumbercount <= std::numeric_limits<uint8_t>::max(),
        "too many messages in _PreparePacketHeader");

    tofill << static_cast<uint8_t>(messagenumbercount);
}

DLLEXPORT void WireData::FillHeaderAckData(const NetworkAckField* acks, sf::Packet& tofill)
{
    if(!acks) {

        // Set first to be zero which assumes that no data follows
        tofill << uint32_t(0);

    } else {

        // Put into the packet //
        acks->AddDataToPacket(tofill);
    }
}
