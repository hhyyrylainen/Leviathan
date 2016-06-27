// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Common/DataStoring/NamedVars.h"


namespace Leviathan{

//! \brief Centralized variables that AI can use on clients to replicate server behaviour
//!
//! Can be used for example to set the position an npc is walking towards
//! to allow clients interpolate
//! its position better
//! \todo Make the QueuedTasks used in this class completely thread safe in case
//! someone decides to delete lots of variables
class NetworkCache : public ThreadSafe{
public:

    //! \param serverside Set to true when this is a NETWORKED_TYPE_SERVER
    DLLEXPORT NetworkCache(bool serverside);
    DLLEXPORT ~NetworkCache();

    //! \brief Call after creating this object
    DLLEXPORT bool Init(NetworkHandler* owner);

    //! \brief Call before deleting
    //!
    //! Drops all active connections and clears the cache
    DLLEXPORT void Release();

    //! \brief Updates a variable
    //!
    //! \param updatedvalue The value to replace/add
    //! \note This can only be called on the server
    DLLEXPORT bool UpdateVariable(const NamedVariableList &updatedvalue);
        
    //! \brief Removes a variable
    //! \note This can only be called on the server
    //! \todo send remove message to clients
    DLLEXPORT bool RemoveVariable(const std::string &name);

    //! \brief Retrieves a variable
    //!
    //! Can be used both on the client and on the server
    //! \note You may not change the variable.
    //! On the server you can use UpdateVariable to change it.
    DLLEXPORT NamedVariableList* GetVariable(const std::string &name) const;

    //! \brief Script wrapper for GetVariable
    DLLEXPORT ScriptSafeVariableBlock* GetVariableWrapper(const std::string &name);

    //! \brief Script wrapper for UpdateVariable
    DLLEXPORT void SetVariableWrapper(ScriptSafeVariableBlock* variable);

    //! \brief Handles an update packet
    //!
    //! \note Works both on  the client and the server but shoulnd't be called on the server
    //! unless you really know what you are doing.
    DLLEXPORT bool HandleUpdatePacket(NetworkResponseDataForAICacheUpdated* data);

protected:

    //! \brief Called when a variable needs to be updated on the clients
    void _OnVariableUpdated(std::shared_ptr<NamedVariableList> variable, Lock &guard);
        
    // ------------------------------------ //
    
    //! True on the server
    const bool IsServer;

    //! States of the AI variables, exists both on the client and the server
    std::vector<std::shared_ptr<NamedVariableList>> CurrentVariables;

    //! Connections that need updating are now fetched through this
    NetworkHandler* Owner = nullptr;
};

}

