// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "AudioBuffer.h"
#include "ProceduralSound.h"

#include "GUI/JSProxyable.h"

#include "alure2.h"

namespace Leviathan {

//! \brief Small ReferenceCounted wrapper around an audio source
class AudioSource : public GUI::JSProxyable {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    DLLEXPORT AudioSource(alure::Source sourcetowrap);

public:
    DLLEXPORT ~AudioSource();

    // DLLEXPORT inline cAudio::IAudioSource* Get()
    // {
    //     return Source;
    // }

    DLLEXPORT inline bool HasInternalSource() const
    {
        // Returns true if the source has the impl pointer set
        return Source.operator bool();
    }

    // ------------------------------------ //
    DLLEXPORT void Play2D(const Sound::AudioBuffer::pointer& buffer);
    DLLEXPORT void PlayWithDecoder(const Sound::ProceduralSoundData::pointer& data,
        size_t chunksize = 12000, size_t chunkstoqueue = 4);

    DLLEXPORT inline void Resume()
    {
        Source.resume();
    }

    DLLEXPORT inline void Stop()
    {
        Source.stop();
    }

    DLLEXPORT inline void Pause()
    {
        Source.pause();
    }

    //! Also returns true if the source will start playing once a buffer is loaded
    DLLEXPORT inline bool IsPlaying() const
    {
        return Source.isPlayingOrPending();
    }

    //! \param volume 0-1.f is normal range
    DLLEXPORT inline void SetVolume(float volume)
    {
        Source.setGain(volume);
    }

    DLLEXPORT inline void SetGain(float gain)
    {
        Source.setGain(gain);
    }

    DLLEXPORT inline void SetLooping(bool looping)
    {
        Source.setLooping(looping);
    }

    REFERENCE_COUNTED_PTR_TYPE(AudioSource);

private:
    alure::Source Source;

    Sound::AudioBuffer::pointer PlayedBuffer;
};

} // namespace Leviathan
