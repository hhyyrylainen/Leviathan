#pragma once
// ------------------------------------ //
#include "Networking/NetworkMasterServerInterface.h"


namespace Pong{

    class PongMasterNetworking : public Leviathan::NetworkMasterServerInterface{
    public:
        PongMasterNetworking();
        virtual ~PongMasterNetworking();
        
    protected:

    };

}

