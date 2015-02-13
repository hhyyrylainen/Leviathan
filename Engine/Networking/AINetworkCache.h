#pragma once
#ifndef LEVIATHAN_AINETWORKCACHE
#define LEVIATHAN_AINETWORKCACHE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/ThreadSafe.h"
#include "Utility/DataStoring/NamedVars.h"


namespace Leviathan{

    //! \brief Centralized variables that AI can use on clients to replicate server behaviour
    //!
    //! Can be used for example to set the position an npc is walking towards to allow clients interpolate
    //! its position better
    //! \todo Make an npc class that can control which variables are sent to which players
    //! \todo Make the QueuedTasks used in this class completely thread safe in case someone decides to delete lots
    //! of variables
	class AINetworkCache : public ThreadSafe{
	public:

        //! \param serverside Set to true when this is a NETWORKED_TYPE_SERVER
		DLLEXPORT AINetworkCache(bool serverside);
        DLLEXPORT ~AINetworkCache();

        //! \brief Call after creating this object
        DLLEXPORT bool Init();

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
        DLLEXPORT bool RemoveVariable(const wstring &name);


        //! \brief Retrieves a variable
        //!
        //! Can be used both on the client and on the server
        //! \note You may not change the variable. On the server you can use UpdateVariable to change it.
        DLLEXPORT const NamedVariableList* GetVariable(const wstring &name) const;

        //! \brief Registers a new connection to be used
        //!
        //! Also creates a task to send all current variables
        //! \note May only be called on the server
        //! \warning This does not check for duplicates
        DLLEXPORT bool RegisterNewConnection(ConnectionInfo* connection);

        //! \brief Removes a connection
        //! \note Won't close the connection
        DLLEXPORT bool RemoveConnection(ConnectionInfo* connection);
        
        //! Retrieve static instance, may be NULL if Engine is released or not initialized
        DLLEXPORT static AINetworkCache* Get();

    protected:

        //! \brief Called when a variable needs to be updated on the clients
        void _OnVariableUpdated(shared_ptr<NamedVariableList> variable, ObjectLock &guard);
        
        // ------------------------------------ //
        
        //! True on the server
        const bool IsServer;

        //! States of the AI variables, exists both on the client and the server
        std::vector<shared_ptr<NamedVariableList>> CurrentVariables;

        //! Connections that are receiving update notifications. Only used on the server
        std::vector<ConnectionInfo*> ReceivingConnections;

        static AINetworkCache* Staticinstance;
	};

}
#endif
