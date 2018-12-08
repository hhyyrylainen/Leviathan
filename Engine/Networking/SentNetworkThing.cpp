// ------------------------------------ //
#include "SentNetworkThing.h"

#include "NetworkRequest.h"
#include "NetworkResponse.h"

#include "TimeIncludes.h"

using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Leviathan::SentNetworkThing::SentNetworkThing(
    uint32_t packetid, uint32_t messagenumber, RECEIVE_GUARANTEE guarantee) :
    PacketNumber(packetid),
    MessageNumber(messagenumber), Resend(guarantee), RequestStartTime(Time::GetTimeMs64())
{}

DLLEXPORT void SentNetworkThing::ResetStartTime()
{

    RequestStartTime = Time::GetTimeMs64();
}

DLLEXPORT void SentNetworkThing::SetWaitStatus(bool status)
{
    IsDone.store(status ? DONE_STATUS::DONE : DONE_STATUS::FAILED, std::memory_order_release);

    if(Callback)
        (*Callback)(status, *this);
}

DLLEXPORT void Leviathan::SentNetworkThing::OnFinalized(bool succeeded)
{
    if(!succeeded) {

        SetWaitStatus(false);
        return;
    }

    if(ConfirmReceiveTime.load(std::memory_order_consume) == 1) {

        ConfirmReceiveTime.store(Time::GetTimeMs64(), std::memory_order_release);
    }

    // We want to notify all waiters that it has been received //
    SetWaitStatus(true);
}

DLLEXPORT bool SentNetworkThing::GetStatus()
{

    while(IsDone.load(std::memory_order_acquire) == DONE_STATUS::WAITING) {

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return IsDone == DONE_STATUS::DONE;
}

DLLEXPORT void SentNetworkThing::SetAsTimed()
{

    ConfirmReceiveTime.store(1, std::memory_order_release);
}

DLLEXPORT void SentNetworkThing::SetCallback(std::shared_ptr<CallbackType> func)
{
    Callback = func;
}

DLLEXPORT void Leviathan::SentNetworkThing::SetCallbackFunc(CallbackType func)
{

    Callback = std::make_shared<CallbackType>(std::move(func));
}

// ------------------------------------ //
// SentRequest
DLLEXPORT SentRequest::SentRequest(uint32_t sentinpacket, uint32_t messagenumber,
    RECEIVE_GUARANTEE guarantee, const std::shared_ptr<NetworkRequest>& request) :
    SentNetworkThing(sentinpacket, messagenumber, guarantee),
    SentRequestData(request)
{}

DLLEXPORT std::string SentRequest::GetTypeStr()
{
    if(SentRequestData) {
        return SentRequestData->GetTypeStr();
    } else {
        return "null";
    }
}

// ------------------------------------ //
// SentRequest
DLLEXPORT SentResponse::SentResponse(uint32_t sentinpacket, uint32_t messagenumber,
    RECEIVE_GUARANTEE guarantee, const std::shared_ptr<NetworkResponse>& response) :
    SentNetworkThing(sentinpacket, messagenumber, guarantee),
    SentResponseData(response), StoredType(static_cast<int>(response->GetType()))
{}

DLLEXPORT SentResponse::SentResponse(
    uint32_t sentinpacket, uint32_t messagenumber, const NetworkResponse& response) :
    SentNetworkThing(sentinpacket, messagenumber, RECEIVE_GUARANTEE::None),
    StoredType(static_cast<int>(response.GetType()))
{}

DLLEXPORT std::string SentResponse::GetTypeStr()
{
    // A bit hacky way to call the member function
    return ResponseNone(static_cast<NETWORK_RESPONSE_TYPE>(StoredType), 0).GetTypeStr();
}
