// ------------------------------------ //
#include "AudioBuffer.h"

#include "SoundDevice.h"

using namespace Leviathan;
using namespace Leviathan::Sound;
// ------------------------------------ //
DLLEXPORT AudioBuffer::AudioBuffer(alure::Buffer buffertowrap, SoundDevice* owner) :
    Buffer(buffertowrap), Owner(owner)
{
    LEVIATHAN_ASSERT(Owner, "AudioBuffer must be associated with a SoundDevice");
}

DLLEXPORT AudioBuffer::~AudioBuffer()
{
    // TODO: should this invoke if not called on the main thread?
    if(Buffer) {

        Owner->ReportDestroyedBuffer(*this);
    }
}
