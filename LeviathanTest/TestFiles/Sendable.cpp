#include "PartialEngine.h"

#include "Entities/GameWorld.h"
#include "Entities/Components.h"
#include "Common/SFMLPackets.h"
#include "Entities/CommonStateObjects.h"
#include "Networking/NetworkResponse.h"


#include "catch.hpp"

using namespace Leviathan;
using namespace std;

TEST_CASE("Sendable get correct server states", "[entity, networking]"){

    PartialEngine<false> engine;


    GameWorld world(NETWORKED_TYPE::Client);
    world.Init(nullptr, nullptr);



    
    world.Release();
}


TEST_CASE("World interpolation system works with Brush", "[entity, networking]"){

}

TEST_CASE("GameWorld properly loads and applies state packets", "[networking, entity]"){

    
}
