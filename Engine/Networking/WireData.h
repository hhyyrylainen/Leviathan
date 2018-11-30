// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

#include "CommonNetwork.h"
#include <functional>
#include <memory>
#include <vector>

namespace sf {
class Packet;
}

namespace Leviathan {

class SentRequest;
class SentResponse;
class NetworkRequest;
class NetworkResponse;
class SentNetworkThing;

class NetworkAckField;

//! Class for serializing and deserializing the final bytes that are
//! sent over the network
//!
//! Used by Connection and tests to format NetworkResponse and NetworkRequest objects
class WireData final {
public:
    //! Return value for controlling how DecodeIncomingData continues after the callback
    enum class DECODE_CALLBACK_RESULT {

        Continue,
        Error
    };

    //! \brief Constructs a single request message containing normal packet
    //! \param bytesreceiver The packet to fill in with the final data.
    //! This will be cleared before data is added so you can reuse the same packet
    //! \param acks The acks to include in the packet header or null for no acks
    //! \param request The request to format
    //! \param guarantee This is just passed on to the result object
    //! \param messagenumber The unique message id for request
    //! \param localpacketid The unique id for the final network packet
    DLLEXPORT static std::shared_ptr<SentRequest> FormatRequestBytes(
        const std::shared_ptr<NetworkRequest>& request, RECEIVE_GUARANTEE guarantee,
        uint32_t messagenumber, uint32_t localpacketid, const NetworkAckField* acks,
        sf::Packet& bytesreceiver);

    //! \brief Constructs a request without creating a SentRequest
    //!
    //! This is used for resends
    DLLEXPORT static void FormatRequestBytes(const NetworkRequest& request,
        uint32_t messagenumber, uint32_t localpacketid, const NetworkAckField* acks,
        sf::Packet& bytesreceiver);

    //! \brief Constructs a single response message
    //! \see FormatRequestBytes
    DLLEXPORT static std::shared_ptr<SentResponse> FormatResponseBytes(
        const std::shared_ptr<NetworkResponse>& response, RECEIVE_GUARANTEE guarantee,
        uint32_t messagenumber, uint32_t localpacketid, const NetworkAckField* acks,
        sf::Packet& bytesreceiver);

    //! \brief Constructs a single response message that can't be resent
    //! \see FormatRequestBytes
    //! \todo This has a lot of duplicated code with FormatResponseBytes
    DLLEXPORT static std::shared_ptr<SentResponse> FormatResponseBytesTracked(
        const NetworkResponse& response, uint32_t messagenumber, uint32_t localpacketid,
        const NetworkAckField* acks, sf::Packet& bytesreceiver);

    //! \brief Constructs a single response message
    //! \note This version is meant for unreliable responses as this doesn't return a
    //! SentResponse. This is also used for resends
    //! \see FormatRequestBytes
    DLLEXPORT static void FormatResponseBytes(const NetworkResponse& response,
        uint32_t messagenumber, uint32_t localpacketid, const NetworkAckField* acks,
        sf::Packet& bytesreceiver);


    //! \brief Constructs an ack only packet with the specified acks
    DLLEXPORT static void FormatAckOnlyPacket(
        const std::vector<uint32_t>& packetstoack, sf::Packet& bytesreceiver);



    //! \brief Decodes a packet to the right objects and invokes the callbacks
    //!
    //! This does the opposito of the various Format methods in this class.
    //! \note In case of errors they will be logged and this will silently return
    //! without invoking the callbacks
    //! \param ackcallback Called when a NetworkAckField is decoded from the packet
    //! \param singleack Called when a a single whole ack number is loaded
    //! \param packetnumberreceived Called when a packet id is decoded
    //! \param handlemessage This is the most important callback as it is called
    //! once for every message decoded from the packet. This may not be null
    //! \param messagereceived Called once for every message. The actual message data
    //! is still in the packet and needs to be decoded. The callback parameters are:
    //! message type and message number
    DLLEXPORT static void DecodeIncomingData(sf::Packet& packet,
        const std::function<DECODE_CALLBACK_RESULT(NetworkAckField&)>& ackcallback,
        const std::function<void(uint32_t)>& singleack,
        const std::function<DECODE_CALLBACK_RESULT(uint32_t)>& packetnumberreceived,
        const std::function<DECODE_CALLBACK_RESULT(uint8_t, uint32_t, sf::Packet&)>&
            messagereceived);



    //! \protected Not meant to be called directly
    //!
    //! used to format packet header fields
    //! \param firstmessagenumber Pointer to first message number
    //! \param messagenumbercount Number of message numbers in firstmessagenumber
    DLLEXPORT static void PrepareHeaderForPacket(uint32_t localpacketid,
        uint32_t* firstmessagenumber, size_t messagenumbercount, const NetworkAckField* acks,
        sf::Packet& tofill);

    //! \protected Format ack part of header
    //!
    //! used by PrepareHeaderForPacket. Split out for testing purposes
    DLLEXPORT static void FillHeaderAckData(const NetworkAckField* acks, sf::Packet& tofill);
};


} // namespace Leviathan
