// ------------------------------------ //
#include "AudioBuffer.h"

#include "SoundDevice.h"

#include "Engine.h"

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
    if(!Buffer)
        return;

    if(Engine::Get()->IsOnMainThread()) {

        Owner->ReportDestroyedBuffer(Buffer);

    } else {
        Engine::Get()->Invoke([bufferCopy = Buffer, device = Owner]() mutable {
            device->ReportDestroyedBuffer(bufferCopy);
        });
    }

    Buffer = nullptr;
}
