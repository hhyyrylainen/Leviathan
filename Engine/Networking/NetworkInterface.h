#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "CommonNetwork.h"
#include "Common/ThreadSafe.h"

#include <memory>


namespace Leviathan{


	//! \brief Class that encapsulates common networking functionality that is required by
//! all networked programs
//! \see NetworkServerInterface NetworkClientInterface
class NetworkInterface : public virtual ThreadSafe{
    friend NetworkHandler;
public:
    DLLEXPORT NetworkInterface(NETWORKED_TYPE type);
    DLLEXPORT virtual ~NetworkInterface();

    //! \brief Called by ConnectionInfo to handle incoming packets
    //!
    //! This function is responsible for interpreting the packet data and generating a
    //! response. If the response could take a long time to generate it is recommended to
    //! queue a task to the ThreadingManager. 
    //! \note The connection parameter shouldn't be stored directly
    //! since it can become invalid after this function returns. Get a shared pointer
    //! from NetworkHandler instead
    DLLEXPORT virtual void HandleRequestPacket(std::shared_ptr<NetworkRequest> request,
        Connection &connection);
        
    //! \brief Called by ConnectionInfo to verify that a response is good.
    //!
    //! By default this will always return true, but that can be overloaded to return false
    //! if the response is no good
    //! When returning false the connection will pretend that the response never arrived
    //! and possibly resends the request
    DLLEXPORT virtual bool PreHandleResponse(std::shared_ptr<NetworkResponse> response,
        SentNetworkThing* originalrequest, Connection &connection);

        
    //! \brief Called by ConnectionInfo when it receives a response
    //! without a matching request object
    //!
    //! This is called when the host on the connection sends a response without a
    //! matching request.
    //! Usually the other program instance wants us to do something
    //! without expecting a response,
    //! for example they could want us to add a new message to our inbox without
    //! expecting a response (other than an ack which is automatically sent) from us. 
    //! The function can optionally ignore keepalive acks (to reduce spam between clients)
    //! by setting dontmarkasreceived as true.
    //! This function shouldn't throw any exceptions.
    DLLEXPORT virtual void HandleResponseOnlyPacket(
        std::shared_ptr<NetworkResponse> message, Connection &connection,
        bool &dontmarkasreceived) = 0;

        
    //! \brief Called by Connection just before terminating an inactive connection
    //!
    //! Return true if you want to allow the connection to close
    //! (it will close before next packet receive update)
    //! By default will allow all requesting connections to terminate.
    DLLEXPORT virtual bool CanConnectionTerminate(Connection &connection);


    //! \brief Should be used to update various network interfaces
    //! \note This NetworkInterface doesn't need any ticking,
    //! but NetworkClientInterface does
    //! \see NetworkClientInterface::UpdateClientStatus
    DLLEXPORT virtual void TickIt();


    //! \brief Called when the program is closing
    //!
    //! This should be used to call network interface type based functions
    //! (NetworkServerInterface::CloseDownServer() etc.)
    DLLEXPORT virtual void CloseDown() = 0;

    //! \brief Asserts if types don't match
    DLLEXPORT void VerifyType(NETWORKED_TYPE type) const;
		
protected:

    //! \brief Utility function for subclasses to call for default handling
    //!
    //! Handles default types of request packages and returns true if processed.
    DLLEXPORT bool _HandleDefaultRequest(std::shared_ptr<NetworkRequest> request,
        Connection &connectiontosendresult);
        
    //! \brief Utility function for subclasses to call for default handling of
    //! non-request responses
    //!
    //! Handles default types of response packages and returns true if processed.
    DLLEXPORT bool _HandleDefaultResponseOnly(std::shared_ptr<NetworkResponse> message,
        Connection &connection, bool &dontmarkasreceived);

    DLLEXPORT void SetOwner(NetworkHandler* owner);
        
    // ------------------------------------ //

    NETWORKED_TYPE OurNetworkType;
    NetworkHandler* Owner = nullptr;
};

}

