// ------------------------------------ //
#include "ProceduralSound.h"

using namespace Leviathan;
using namespace Leviathan::Sound;
// ------------------------------------ //
DLLEXPORT ProceduralSoundData::ProceduralSoundData(
    std::function<unsigned(void*, unsigned)> datacallback, const SoundProperties& properties) :
    Properties(properties),
    DataCallback(datacallback)
{}

DLLEXPORT ProceduralSoundData::~ProceduralSoundData() {}

DLLEXPORT void ProceduralSoundData::Detach()
{
    GUARD_LOCK();

    Detached = true;
}
// ------------------------------------ //
DLLEXPORT unsigned ProceduralSoundData::ReadAudioData(void* output, unsigned amount)
{
    return DataCallback(output, amount);
}
