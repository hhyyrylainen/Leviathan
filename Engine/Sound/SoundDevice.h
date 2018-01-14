// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Common/Types.h"
#include <memory>
#include <vector>
// #include "SoundPlayingSlot.h"


// #define SOUND_UNLOAD_UNUSEDTIME		30000
// #define MAX_CONCURRENT_SOUNDS		256

namespace Leviathan{

class SoundDevice{
public:
    DLLEXPORT SoundDevice();
    DLLEXPORT ~SoundDevice();

    DLLEXPORT bool Init();
    DLLEXPORT void Release();

    DLLEXPORT void Tick(int PassedMs);

    //! \brief Loads the file and plays the sound
    //!
    //! This creates a temporary SoundEffect on a background thread that is loaded
    //! from the file and destroyed once it finishes
    //! \returns False if the file doesn't exist or the sound couldn't be played for
    //! some other reason
    DLLEXPORT bool PlaySoundEffect(const std::string &file);

    DLLEXPORT void SetSoundListenerPosition(const Float3 &pos, const Float4 &orientation);
    DLLEXPORT void SetGlobalVolume(const float &vol);
    // Getting proper sound stream functions //

    // DLLEXPORT std::shared_ptr<SoundPlayingSlot> GetSlotForSound(const std::string &file);
    // // used for streams //
    // DLLEXPORT std::shared_ptr<SoundPlayingSlot> GetSlotForSound();

    DLLEXPORT static SoundDevice* Get();
private:



    // ------------------------------------ //
    //std::vector<std::shared_ptr<SoundPlayingSlot>> LoadedSoundObjects;


    static SoundDevice* Instance;
};

}

