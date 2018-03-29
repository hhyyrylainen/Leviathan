// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include "cAudio/IAudioSource.h"

namespace cAudio {

class IAudioSource;
}

namespace Leviathan {

class SoundDevice;

//! \brief Small ReferenceCounted wrapper around a sound stream
class AudioSource : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    DLLEXPORT AudioSource(cAudio::IAudioSource* sourcetowrap, SoundDevice* owner);

public:
    DLLEXPORT ~AudioSource();

    DLLEXPORT inline cAudio::IAudioSource* Get()
    {
        return Source;
    }

    // ------------------------------------ //
    // Proxies for some of the common audio functions
    DLLEXPORT inline void Play()
    {
        if(Source)
            Source->play();
    }

    DLLEXPORT inline void Stop()
    {
        if(Source)
            Source->stop();
    }

    REFERENCE_COUNTED_PTR_TYPE(AudioSource);

private:
    cAudio::IAudioSource* Source;

    //! Used to release the source properly
    SoundDevice* Owner;
};

} // namespace Leviathan
