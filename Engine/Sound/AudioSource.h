// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GUI/JSProxyable.h"

#include "cAudio/IAudioSource.h"

namespace cAudio {

class IAudioSource;
}

namespace Leviathan {

class SoundDevice;

//! \brief Small ReferenceCounted wrapper around a sound stream
class AudioSource : public GUI::JSProxyable {
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

    DLLEXPORT inline bool HasInternalSource() const
    {
        return Source != nullptr;
    }

    // ------------------------------------ //
    // Proxies for some of the common audio functions
    DLLEXPORT inline bool Play2D()
    {
        if(Source)
            return Source->play2d();
        return false;
    }

    DLLEXPORT inline void Stop()
    {
        if(Source)
            Source->stop();
    }

    DLLEXPORT inline void Pause()
    {
        if(Source)
            Source->pause();
    }

    DLLEXPORT inline bool IsPlaying() const
    {
        if(Source)
            return Source->isPlaying();
        return false;
    }

    //! \param volume 0-1.f is normal range
    DLLEXPORT inline void SetVolume(float volume)
    {
        if(Source)
            Source->setVolume(volume);
    }

    REFERENCE_COUNTED_PTR_TYPE(AudioSource);

private:
    cAudio::IAudioSource* Source;

    //! Used to release the source properly
    SoundDevice* Owner;
};

} // namespace Leviathan
