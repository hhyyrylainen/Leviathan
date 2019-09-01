// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "AudioBuffer.h"
#include "AudioSource.h"
#include "ProceduralSound.h"

#include "Common/Types.h"

namespace Leviathan {

//! \brief Manages loading the audio library and provides some helpers
class SoundDevice {
    struct Implementation;

public:
    DLLEXPORT SoundDevice();
    DLLEXPORT ~SoundDevice();

    //! \param simulatenosound If true the sound device isn't initialized to simulate not
    //! having a valid audio device (or if the user just doesn't want sound)
    DLLEXPORT bool Init(bool simulatesound = false);
    DLLEXPORT void Release();

    DLLEXPORT void Tick(int PassedMs);

    //! \brief Loads the file and plays the sound
    //!
    //! This creates a temporary SoundEffect on a background thread that is loaded
    //! from the file and destroyed once it finishes
    //! \returns False if the file doesn't exist or the sound couldn't be played for
    //! some other reason
    // DLLEXPORT bool PlaySoundEffect(const std::string& file);

    DLLEXPORT void SetSoundListenerPosition(const Float3& pos, const Float4& orientation);

    //! \param vol The volume [0.f, 1.f]
    DLLEXPORT void SetGlobalVolume(float vol);


    // ------------------------------------ //
    // Audio playing functions

    //! \brief Plays a 2d sound without possibility of interrupting
    DLLEXPORT void Play2DSoundEffect(const std::string& filename);

    //! \brief Plays a 2d sound with options
    //! \returns The audio source that is playing the sound (this must be held onto until it is
    //! done playing, can be passed to BabysitAudio if not manually wanted to be managed or use
    //! the SoundEffect variant of this method) may be null on error
    DLLEXPORT AudioSource::pointer Play2DSound(const std::string& filename, bool looping);

    //! \brief Opens an audio source from a procedural data stream
    DLLEXPORT AudioSource::pointer CreateProceduralSound(
        const Sound::ProceduralSoundData::pointer& data, size_t chunksize = 56000,
        size_t chunkstoqueue = 4);

    //! \brief Creates a sound buffer from a file
    DLLEXPORT Sound::AudioBuffer::pointer GetBufferFromFile(
        const std::string& filename, bool cache = true);

    //! \brief Creates an audio source with no settings applied
    DLLEXPORT AudioSource::pointer GetAudioSource();

    //! \brief This class holds the audio source until it has finished
    //! playing and then releases the reference
    DLLEXPORT void BabysitAudio(AudioSource::pointer audio);

    // ------------------------------------ //
    DLLEXPORT void ReportDestroyedBuffer(Sound::AudioBuffer& buffer);

private:
    std::unique_ptr<Implementation> Pimpl;

    int CacheSoundEffectMilliseconds = 30000;
    int ElapsedSinceLastClean = 0;
};

} // namespace Leviathan
