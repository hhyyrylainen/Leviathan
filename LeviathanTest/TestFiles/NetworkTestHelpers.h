#pragma once

#include "Networking/NetworkClientInterface.h"
#include "Networking/NetworkServerInterface.h"
#include "Networking/Connection.h"

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

class GapingConnectionTest : public Connection{
public:

    GapingConnectionTest(unsigned int port) : Connection(sf::IpAddress::LocalHost, port)
    {
        State = CONNECTION_STATE::Authenticated;
    }

};

}
