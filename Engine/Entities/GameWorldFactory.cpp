// ------------------------------------ //
#include "GameWorldFactory.h"

#include "Generated/StandardWorld.h"
using namespace Leviathan;
// ------------------------------------ //

GameWorldFactory::GameWorldFactory(){

    LEVIATHAN_ASSERT(StaticInstance == nullptr, "Multiple GameWorldFactories created");
    StaticInstance = this;
}

GameWorldFactory::~GameWorldFactory(){

    StaticInstance = nullptr;
}

GameWorldFactory* GameWorldFactory::StaticInstance = nullptr;

DLLEXPORT GameWorldFactory* GameWorldFactory::Get(){
    return StaticInstance;
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<GameWorld> GameWorldFactory::CreateNewWorld(){

    return std::make_shared<StandardWorld>();
}

