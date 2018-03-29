// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "AudioSource.h"
#include "ProceduralSound.h"

#include "Common/Types.h"

namespace cAudio {

class IAudioManager;
class IListener;
} // namespace cAudio

namespace Leviathan {

//! \brief Manages loading the audio library and provides some helpers
class SoundDevice {
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

    //! \brief Opens an audio source from a procedural data stream
    //! \param soundname Name for this audio source. Should be at least somewhat unique
    DLLEXPORT AudioSource::pointer CreateProceduralSound(
        ProceduralSoundData::pointer data, const char* soundname);


    // ------------------------------------ //
    DLLEXPORT inline cAudio::IAudioManager* GetAudioManager()
    {
        return AudioManager;
    }

    //! \brief Returns a list of audio playback devices
    //! \param indexofdefault Returns the index of the default device (if not null)
    DLLEXPORT static std::vector<std::string> GetAudioDevices(
        size_t* indexofdefault = nullptr);

private:
    cAudio::IAudioManager* AudioManager = nullptr;
    cAudio::IListener* ListeningPosition = nullptr;

    //! Needs to be kept around as cAudio doesn't copy the string
    std::string AudioLogPath;
};

} // namespace Leviathan
