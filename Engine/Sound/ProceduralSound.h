// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "Common/ThreadSafe.h"

#include "cAudio/EAudioFormats.h"

#include <atomic>
#include <map>

namespace Leviathan {

class SoundDevice;

//! \brief The main usable class for doing procedural audio
//!
//! This can be passed to SoundDevice to get a playing stream that plays this data
//! \note The stream will end if the callback doesn't return any data (so if you expect more
//! data later return couple thousand 0s)
//! \todo Move all the other classes from this file to SoundInternalTypes.h
class ProceduralSoundData : public ReferenceCounted, public ThreadSafe {
    friend SoundDevice;

public:
    struct SoundProperties {
        //! Format that the callback needs to generate
        //!
        //! Recommended formats:
        //! cAudio::EAF_16BIT_STEREO
        //! cAudio::EAF_16BIT_MONO
        cAudio::AudioFormats Format;

        //! The number of samples per second
        int SampleRate;

        //! The name of this data source for example (VideoPlayer)
        std::string SourceName;
    };

protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    //! \see readAudioData for details about the datacallback
    DLLEXPORT ProceduralSoundData(
        std::function<int(void*, int)> datacallback, SoundProperties&& properties);

public:
    DLLEXPORT ~ProceduralSoundData();

    //! Reads a section of data out of the audio stream by running the callback
    //! \param output Pointer to the buffer to put the decoded audio
    //! \param amount Amount of data in bytes to ask the decoder to output. This is also the
    //! size of output
    //! \returns Number of bytes of audio data actually written to output
    DLLEXPORT inline int ReadAudioData(void* output, int amount)
    {
        GUARD_LOCK();

        return DataCallback(output, amount);
    }

    DLLEXPORT inline bool IsValid() const
    {
        return !Detached;
    }

    //! \brief Permanently detaches this source from the callback.
    //!
    //! After this call finishes the callback won't be called again
    DLLEXPORT void Detach();

    REFERENCE_COUNTED_PTR_TYPE(ProceduralSoundData);

    //! Properties for the ProceduralSoundStream to retrieve when needed
    const SoundProperties Properties;

protected:
    //! If set to true then this will no longer call the data callback.
    //! This can be used to safely let go of a procedural audio before destroying the whatever
    //! source that DataCallback uses
    std::atomic<bool> Detached = false;

    //! Called each time the underlying audio stream requests more data
    std::function<int(void*, int)> DataCallback;
};

// ------------------------------------ //


} // namespace Leviathan
