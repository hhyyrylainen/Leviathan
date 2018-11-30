// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Gameplay/CommandHandler.h"

#include "TimeIncludes.h"

#include <string>

namespace Leviathan {

//! \brief A class that represents a human player
//! \todo Add a kick method and use it in NetworkServerInterface::CloseDownServer
class ConnectedPlayer : public CommandSender {
public:
    DLLEXPORT ConnectedPlayer(std::shared_ptr<Connection> connection,
        NetworkServerInterface* owninginstance, int plyid);

    //! \brief Empty destructor for exporting
    DLLEXPORT ~ConnectedPlayer();

    //! \brief Checks is the given connection same as ours
    DLLEXPORT inline bool IsConnectionYours(Connection* checkconnection)
    {
        return CorrespondingConnection.get() == checkconnection;
    }

    //! \returns True if this player's connection is closed and this object should be
    //! disposed of
    DLLEXPORT bool IsConnectionClosed() const;

    //! \brief Call this when the player is kicked
    //! \todo Add the reason to the packet
    DLLEXPORT void OnKicked(const std::string& reason);

    //! \brief Starts requiring the player to send heartbeats
    DLLEXPORT void StartHeartbeats();

    //! \brief Call this when a heartbeat is received
    DLLEXPORT void HeartbeatReceived();

    //! \brief Call this at any appropriate time to update heartbeat statistics
    DLLEXPORT void UpdateHeartbeats();

    inline const std::shared_ptr<Connection>& GetConnection()
    {
        return CorrespondingConnection;
    }

    //! \brief Gets the unique identifier of the player, valid for this session
    inline int GetID() const
    {
        return ID;
    }

    //! \brief Returns the object that contains this players position in a certain world or
    //! 0
    DLLEXPORT ObjectID GetPositionInWorld(GameWorld* world) const;


    const std::string& GetUniqueName() override
    {
        return UniqueName;
    }

    const std::string& GetNickname() override
    {
        return DisplayName;
    }

    DLLEXPORT COMMANDSENDER_PERMISSIONMODE GetPermissionMode() override;

protected:
    DLLEXPORT bool _OnSendPrivateMessage(const std::string& message) override;
    // ------------------------------------ //

    std::shared_ptr<Connection> CorrespondingConnection;
    NetworkServerInterface* Owner;

    //! Marks whether heartbeats are in use
    bool UsingHeartbeats;

    //! The unique identifier of the player, might be a steam id or something else
    std::string UniqueName;

    //! The display name of the player
    std::string DisplayName;

    //! Gets set when a heartbeat hasn't been received for a while,
    //! this will be set before the player is kicked
    bool IsControlLost;

    //! How long it has been since a heartbeat
    float SecondsWithoutConnection;

    //! The last time a heartbeat was sent
    WantedClockType::time_point LastSentHeartbeat;

    //! The last time a heartbeat packet was received
    WantedClockType::time_point LastReceivedHeartbeat;

    //! The unique identifier for this player, lasts only this session
    int ID;
};

} // namespace Leviathan
