// ------------------------------------ //
#include "AudioSource.h"

#include "Engine.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT AudioSource::AudioSource(alure::Source sourcetowrap) : Source(sourcetowrap) {}

DLLEXPORT AudioSource::~AudioSource()
{
    if(Source) {

        if(Engine::Get()->IsOnMainThread()) {

            Source.destroy();

        } else {
            Engine::Get()->Invoke([source = Source]() mutable { source.destroy(); });
        }

        Source = nullptr;
    }

    PlayedBuffer.reset();
}
// ------------------------------------ //
DLLEXPORT void AudioSource::Play2D(const Sound::AudioBuffer::pointer& buffer)
{
    Engine::Get()->AssertIfNotMainThread();

    PlayedBuffer = buffer;

    if(PlayedBuffer && PlayedBuffer->GetBuffer()) {

        Source.set3DSpatialize(alure::Spatialize::Off);
        Source.play(PlayedBuffer->GetBuffer());

    } else {
        Source.stop();
    }
}

DLLEXPORT void AudioSource::PlayWithDecoder(
    const Sound::ProceduralSoundData::pointer& data, size_t chunksize, size_t chunkstoqueue)
{
    Engine::Get()->AssertIfNotMainThread();

    PlayedBuffer.reset();

    if(data) {

        Source.play(data, chunksize, chunkstoqueue);

    } else {
        Source.stop();
    }
}
