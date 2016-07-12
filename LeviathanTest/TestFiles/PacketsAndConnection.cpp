#include "Networking/Connection.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkRequest.h"

#include "PartialEngine.h"

#include "catch.hpp"

using namespace std;
using namespace Leviathan;

class TestClientInterface : public NetworkClientInterface {
public:
    
    virtual void HandleResponseOnlyPacket(std::shared_ptr<NetworkResponse> message, 
        Connection &connection, bool &dontmarkasreceived) override 
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
        Connection &connection, bool &dontmarkasreceived) override 
    {
    }
};

TEST_CASE("Ack field filling", "networking") {

    // We cannot test any client or server specific code,
    // so hopefully using master disables those
    PartialEngine<false> engine;

    Connection conn(sf::IpAddress::LocalHost, 256);



}

TEST_CASE("Connect to localhost socket", "networking"){

    PartialEngine<false> engine;


    TestClientInterface ClientInterface;
    NetworkHandler Client(NETWORKED_TYPE::Client, &ClientInterface);
    
    TestServerInterface ServerInterface;
    NetworkHandler Server(NETWORKED_TYPE::Server, &ServerInterface);

}



