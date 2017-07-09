// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once

#include "Networking/NetworkClientInterface.h"
#include "Networking/NetworkServerInterface.h"
#include "Networking/Connection.h"

#include "PartialEngine.h"

namespace Leviathan{
namespace Test{

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

//! Test fixture for testing client to server messaging
class ConnectionTestFixture{
protected:

    ConnectionTestFixture() : 
        Client(NETWORKED_TYPE::Client, &ClientInterface),
        Server(NETWORKED_TYPE::Server, &ServerInterface)
    {

        REQUIRE(Client.Init(sf::Socket::AnyPort));
        REQUIRE(Server.Init(sf::Socket::AnyPort));

        ClientConnection = std::make_shared<Connection>(
            sf::IpAddress::LocalHost, Server.GetOurPort());

        ServerConnection = std::make_shared<Connection>(
            sf::IpAddress::LocalHost, Client.GetOurPort());

        Server._RegisterConnection(ServerConnection);
        Client._RegisterConnection(ClientConnection);

        ClientConnection->Init(&Client);
        ServerConnection->Init(&Server);

        CHECK(ClientConnection->GetState() == CONNECTION_STATE::NothingReceived);
        CHECK(ServerConnection->GetState() == CONNECTION_STATE::NothingReceived);
    }

    void RunListeningLoop(int times = 3) {
        
        for (int i = 0; i < times; ++i) {

            Server.UpdateAllConnections();
            Client.UpdateAllConnections();
        }
    }

protected:

    PartialEngine<false> engine;

    TestClientInterface ClientInterface;
    NetworkHandler Client;


    TestServerInterface ServerInterface;
    NetworkHandler Server;

    std::shared_ptr<Connection> ClientConnection;
    std::shared_ptr<Connection> ServerConnection;
};


//! Test fixture for testing client sent messages to a raw socket
class ClientConnectionTestFixture{
protected:

    ClientConnectionTestFixture() : 
        Client(NETWORKED_TYPE::Client, &ClientInterface)
    {
        
        RawSocket.setBlocking(false);
        REQUIRE(RawSocket.bind(sf::Socket::AnyPort) == sf::Socket::Done);

        REQUIRE(Client.Init(sf::Socket::AnyPort));

        ClientConnection = std::make_shared<Connection>(
            sf::IpAddress::LocalHost, RawSocket.getLocalPort());

        Client._RegisterConnection(ClientConnection);

        ClientConnection->Init(&Client);

        CHECK(ClientConnection->GetState() == CONNECTION_STATE::NothingReceived);
    }

    //! Does the standard connection opening, after this methods on
    //! the client can be tested to see what packets they send
    void DoConnectionOpening(){

        
        REQUIRE(ClientConnection->GetState() == CONNECTION_STATE::Authenticated);
    }

    void RunListeningLoop(int times = 1) {
        
        for (int i = 0; i < times; ++i) {

            Client.UpdateAllConnections();
        }
    }

protected:

    PartialEngine<false> engine;

    TestClientInterface ClientInterface;
    NetworkHandler Client;

    std::shared_ptr<Connection> ClientConnection;

    sf::UdpSocket RawSocket;
};

}}
