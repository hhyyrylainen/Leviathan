// ------------------------------------ //
#include "AudioSource.h"

#include "SoundDevice.h"

#include "cAudio/cAudio.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT AudioSource::AudioSource(cAudio::IAudioSource* sourcetowrap, SoundDevice* owner) :
    Source(sourcetowrap), Owner(owner)
{

    LEVIATHAN_ASSERT(Owner, "AudioSource must be associated with a SoundDevice");
}

DLLEXPORT AudioSource::~AudioSource()
{
    // TODO: should this invoke if not called on the main thread
    if(Source) {

        auto manager = Owner->GetAudioManager();
        if(manager)
            manager->release(Source);
    }
}
