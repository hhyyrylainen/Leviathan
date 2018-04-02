// ------------------------------------ //
#include "ProceduralSound.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT ProceduralSoundData::ProceduralSoundData(
    std::function<int(void*, int)> datacallback, SoundProperties&& properties) :
    Properties(properties),
    DataCallback(datacallback)
{
}

DLLEXPORT ProceduralSoundData::~ProceduralSoundData() {}

DLLEXPORT void ProceduralSoundData::Detach()
{
    GUARD_LOCK();

    Detached = true;
}
