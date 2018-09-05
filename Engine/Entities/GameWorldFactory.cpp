// ------------------------------------ //
#include "GameWorldFactory.h"

#include "Generated/StandardWorld.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT GameWorldFactory::GameWorldFactory()
{
    LEVIATHAN_ASSERT(StaticInstance == nullptr, "Multiple GameWorldFactories created");
    StaticInstance = this;
}

DLLEXPORT GameWorldFactory::~GameWorldFactory()
{
    StaticInstance = nullptr;
}

DLLEXPORT GameWorldFactory* GameWorldFactory::StaticInstance = nullptr;

DLLEXPORT GameWorldFactory* GameWorldFactory::Get()
{
    return StaticInstance;
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<GameWorld> GameWorldFactory::CreateNewWorld(int worldtype)
{
    if(worldtype != 0)
        LOG_WARNING("Standard GameWorldFactory: worldtype is not 0");

    return std::make_shared<StandardWorld>();
}
// ------------------------------------ //
// InbuiltWorldFactory
std::shared_ptr<GameWorld> InbuiltWorldFactory::CreateNewWorld(INBUILT_WORLD_TYPE worldtype)
{
    switch(worldtype) {
    case INBUILT_WORLD_TYPE::Standard: return std::make_shared<StandardWorld>();
    }

    return nullptr;
}
