// ------------------------------------ //
#include "AudioSource.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT AudioSource::AudioSource(alure::Source sourcetowrap) : Source(sourcetowrap) {}

DLLEXPORT AudioSource::~AudioSource()
{
    // TODO: should this invoke if not called on the main thread?
    if(Source) {
        Source.destroy();
    }
}
// ------------------------------------ //
DLLEXPORT void AudioSource::Play2D(const Sound::AudioBuffer::pointer& buffer)
{
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
    PlayedBuffer.reset();

    if(data) {

        Source.play(data, chunksize, chunkstoqueue);

    } else {
        Source.stop();
    }
}
