// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
//#include "GUI/JSProxyable.h"
#include "Common/ReferenceCounted.h"

#include "alure2.h"

namespace Leviathan {

class SoundDevice;
class AudioSource;

namespace Sound {



//! \brief Small ReferenceCounted wrapper around an audio buffer
class AudioBuffer : public ReferenceCounted {
protected:
    friend SoundDevice;
    friend AudioSource;

    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    DLLEXPORT AudioBuffer(alure::Buffer buffertowrap, SoundDevice* owner);

public:
    DLLEXPORT ~AudioBuffer();

    // ------------------------------------ //

    REFERENCE_COUNTED_PTR_TYPE(AudioBuffer);

protected:
    alure::Buffer& GetBuffer()
    {
        return Buffer;
    }

private:
    alure::Buffer Buffer;

    //! Used to release the source properly
    SoundDevice* Owner;
};

} // namespace Sound
} // namespace Leviathan
