#pragma once

#include "Networking/NetworkClientInterface.h"
#include "Networking/NetworkServerInterface.h"

namespace Leviathan{

class TestClientInterface : public NetworkClientInterface {
public:
    
    virtual void HandleResponseOnlyPacket(std::shared_ptr<NetworkResponse> message, 
        Connection &connection) override 
    {
    }

protected:
    virtual void _OnStartApplicationConnect() override {
    }

};

class TestServerInterface : public NetworkServerInterface {
public:

    TestServerInterface() : NetworkServerInterface(1, "TestServer"){ }
    
    virtual void HandleResponseOnlyPacket(std::shared_ptr<NetworkResponse> message, 
        Connection &connection) override 
    {
    }
};

}
