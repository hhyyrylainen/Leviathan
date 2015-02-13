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
    //! Can be used for example to set the position a npc is walking towards to allow clients interpolate
    //! its position better
	class AINetworkCache : public ThreadSafe{
	public:
		DLLEXPORT AINetworkCache();
        DLLEXPORT ~AINetworkCache();

        //! \brief Call after creating this object
        //! \param serverside Set to true when this is a NETWORKED_TYPE_SERVER
        DLLEXPORT bool Init(bool serverside);

        //! \brief Call before deleting
        //!
        //! Drops all active connections and clears the cache
        DLLEXPORT void Release();

        

    protected:


        //! States of the AI variables, exists both on the client and the server
        std::vector<shared_ptr<NamedVariableList>> CurrentVariables;

        //! Connections that are receiving update notifications. Only used on the server
        std::vector<ConnectionInfo*> ReceivingConnection;
	};

}
#endif
