#include "Networking/Connection.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkRequest.h"

#include "PartialEngine.h"

#include "catch.hpp"

using namespace std;
using namespace Leviathan;

class TestClientInterface : public NetworkClientInterface {
public:
    
    
};

class TestServerInterface : public NetworkServerInterface {
public:
    

};

TEST_CASE("Connect to localhost socket", "networking"){

    PartialEngine<false, NETWORKED_TYPE::Master> engine;


    TestClientInterface ClientInterface;
    NetworkHandler Client;
    
    TestServerInterface ServerInterface;
    NetworkHandler Server;

}


TEST_CASE("Ack field filling", "networking"){

    // We cannot test any client or server specific code,
    // so hopefully using master disables those
    PartialEngine<false, NETWORKED_TYPE_MASTER> engine;

    Connection conn(sf::IpAddress::LocalHost, 256);


    
}
