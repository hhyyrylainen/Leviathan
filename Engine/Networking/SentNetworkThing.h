// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "CommonNetwork.h"

#include <future>
#include <memory>

namespace Leviathan {

//! Represents a sent packet and holds all kinds of data for it
class SentNetworkThing {
public:
    enum class DONE_STATUS {

        WAITING,
        DONE,
        FAILED
    };

    using CallbackType = std::function<void(bool, SentNetworkThing&)>;

    DLLEXPORT SentNetworkThing(
        uint32_t packetid, uint32_t messagenumber, RECEIVE_GUARANTEE guarantee);

    DLLEXPORT ~SentNetworkThing() {}

    //! \brief Returns true once the packet has been received by the target or lost
    //! too many times
    inline bool IsFinalized()
    {
        return IsDone.load(std::memory_order_consume) != DONE_STATUS::WAITING;
    }

    //! \brief Called by Connection once this is done
    DLLEXPORT void OnFinalized(bool succeeded);

    //! \brief Gets the status once IsFinalized returns true blocks otherwise
    //! \return True when the packet has been successfully received, false if lost
    //! \todo Make sure this cannot deadlock
    DLLEXPORT bool GetStatus();

    //! \brief Sets the status of the wait object notifying all waiters that this has
    //! succeeded or failed
    //!
    //! Will also call the Callback if one is set
    //! \note May only be called once
    DLLEXPORT void SetWaitStatus(bool status);

    //! \brief Sets this packet as a timed packet
    //! \note A timed package will have the ConfirmReceiveTime set to the time a response
    //! (or receive notification) is received
    DLLEXPORT void SetAsTimed();

    //! \brief Resets the start time
    DLLEXPORT void ResetStartTime();

    //! \brief Binds a callback function that is called either when the packet is
    //! successfully sent or it times out
    //! \bug This can corrupt the arguments passed to this function, not recommended for use
    DLLEXPORT void SetCallback(std::shared_ptr<CallbackType> func = nullptr);


    DLLEXPORT void SetCallbackFunc(CallbackType func);

    virtual std::string GetTypeStr() = 0;


    //! Contained in Local packet id
    uint32_t PacketNumber;

    //! The ID of the message that was sent (this stays same until this
    //! has failed or succeeded)
    const uint32_t MessageNumber;

    //! If not RECEIVE_GUARANTEE::None this packet will be resent if considered lost
    RECEIVE_GUARANTEE Resend = RECEIVE_GUARANTEE::None;

    //! Used to detect when a critical packet is lost or if this packet has a specific
    //! number of resends
    uint8_t AttemptNumber = 1;

    //! Callback function called when succeeded or failed
    //! May only be called by the receiving thread when removing this
    //! from the queue. May not be changed after settings to make sure
    //! that no race conditions exist
    std::shared_ptr<std::function<void(bool, SentNetworkThing&)>> Callback;

    //! \brief The time when this packed got marked as received
    //!
    //! This will roughly be the time it took for the packet to reach the destination and
    //! return the round-trip time
    //! \note This will only be set if this value is set to 1 before the packet is sent
    //! \note This value is only valid if the packet wasn't lost
    //! (failed requests have this unset)
    std::atomic<int64_t> ConfirmReceiveTime{0};

    //! \brief Time this was started. Used to time out this packet and
    //! calculate round trip time
    int64_t RequestStartTime{0};

    //! Set to true once this object is no longer used
    std::atomic<DONE_STATUS> IsDone{DONE_STATUS::WAITING};
};


//! \brief Stores Requests while they are waiting for a response
class SentRequest : public SentNetworkThing {
public:
    DLLEXPORT SentRequest(uint32_t sentinpacket, uint32_t messagenumber,
        RECEIVE_GUARANTEE guarantee, const std::shared_ptr<NetworkRequest>& request);

    DLLEXPORT std::string GetTypeStr() override;

    std::shared_ptr<NetworkResponse> GotResponse;

    std::shared_ptr<NetworkRequest> SentRequestData;
};

//! \brief Stores Responses that want confirmation that they have arrived
class SentResponse : public SentNetworkThing {
public:
    DLLEXPORT SentResponse(uint32_t sentinpacket, uint32_t messagenumber,
        RECEIVE_GUARANTEE guarantee, const std::shared_ptr<NetworkResponse>& response);

    //! This is a variant that can't be resent
    DLLEXPORT SentResponse(
        uint32_t sentinpacket, uint32_t messagenumber, const NetworkResponse& response);

    DLLEXPORT std::string GetTypeStr() override;

    std::shared_ptr<NetworkResponse> SentResponseData;

    //! Stored for GetTypeStr with non-resendable responses
    int StoredType;
};


} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::SentRequest;
using Leviathan::SentResponse;
#endif
