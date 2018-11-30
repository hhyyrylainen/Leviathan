// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "NetworkInterface.h"
#include "NetworkResponse.h"



namespace Leviathan {


//! \brief Class that encapsulates common networking functionality
//! required by server programs
//!
//! More specific version of NetworkInterface and should be included additionally in
//! server network interface classes.
//! \see NetworkInterface
class NetworkMasterServerInterface : public NetworkInterface {
public:
    DLLEXPORT NetworkMasterServerInterface();
    DLLEXPORT ~NetworkMasterServerInterface();

    DLLEXPORT virtual void HandleRequestPacket(
        const std::shared_ptr<NetworkRequest>& request, Connection& connection) override;

    DLLEXPORT virtual void HandleResponseOnlyPacket(
        const std::shared_ptr<NetworkResponse>& message, Connection& connection) override;

    DLLEXPORT void TickIt() override;


    //! \brief Call this before shutting down the server to kick all players properly
    //! \todo Actually call this, maybe make this an event listener
    DLLEXPORT virtual void CloseDown() override;


    DLLEXPORT virtual std::vector<std::shared_ptr<Connection>>&
        GetClientConnections() override;

protected:
    // ------------------------------------ //
};

} // namespace Leviathan
