// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
// #include "Common/ReferenceCounted.h"
#include "Common/ThreadSafe.h"

#include "alure2.h"

#include <atomic>
#include <map>

namespace Leviathan {

class SoundDevice;

namespace Sound {

//! \brief The main usable class for doing procedural audio
//!
//! This can be passed to SoundDevice to get a playing stream that plays this data
//! \note The stream will end if the callback doesn't return any data (so if you expect more
//! data later return couple thousand 0s)
class ProceduralSoundData : public alure::Decoder, public ThreadSafe {
    friend SoundDevice;

public:
    struct SoundProperties {
        //! Recommended:
        //! alure::ChannelConfig::Mono
        //! alure::ChannelConfig::Stereo
        alure::ChannelConfig Channels;

        //! Recommended: Int16
        alure::SampleType SampleType = alure::SampleType::Int16;

        //! The number of samples per second
        int SampleRate;
    };

    using pointer = alure::SharedPtr<ProceduralSoundData>;

    // protected:
    // // These are protected for only constructing properly reference
    // // counted instances through MakeShared
    // friend ReferenceCounted;

    //! \see readAudioData for details about the datacallback
    DLLEXPORT ProceduralSoundData(std::function<unsigned(void*, unsigned)> datacallback,
        const SoundProperties& properties);

public:
    DLLEXPORT ~ProceduralSoundData();

    //! Reads a section of data out of the audio stream (by default from the callback)
    //! \param output Pointer to the buffer to put the decoded audio
    //! \param amount Amount of data in sample frames (not bytes) to ask the decoder to output.
    //! \returns Number of samples of audio data actually written to output. If less than
    //! amount marks the end of the data.
    DLLEXPORT virtual unsigned ReadAudioData(void* output, unsigned amount);

    DLLEXPORT inline bool IsValid() const
    {
        return !Detached;
    }

    //! \brief Permanently detaches this source from the callback.
    //!
    //! After this call finishes the callback won't be called again
    DLLEXPORT void Detach();

    // alure::Decoder interface
    virtual ALuint getFrequency() const noexcept override
    {
        return Properties.SampleRate;
    }

    virtual alure::ChannelConfig getChannelConfig() const noexcept override
    {
        return Properties.Channels;
    }

    virtual alure::SampleType getSampleType() const noexcept override
    {
        return Properties.SampleType;
    }

    virtual uint64_t getLength() const noexcept override
    {
        // Unknown
        return 0;
    }

    virtual bool seek(uint64_t pos) noexcept override
    {
        return false;
    }

    virtual std::pair<uint64_t, uint64_t> getLoopPoints() const noexcept override
    {
        return {0, 0};
    }

    virtual ALuint read(ALvoid* ptr, ALuint count) noexcept override
    {
        GUARD_LOCK();

        if(Detached)
            return 0;

        try {
            return ReadAudioData(ptr, count);

        } catch(const std::exception& e) {
            LOG_ERROR("Stopping ProceduralSound due to exception: " + std::string(e.what()));
            return 0;
        }
    }

    // REFERENCE_COUNTED_PTR_TYPE(ProceduralSoundData);

    //! Properties for the ProceduralSoundStream to retrieve when needed
    const SoundProperties Properties;

protected:
    //! If set to true then this will no longer call the data callback.
    //! This can be used to safely let go of a procedural audio before destroying the whatever
    //! source that DataCallback uses
    std::atomic<bool> Detached = false;

    //! Called by default in ReadAudioData
    std::function<unsigned(void*, unsigned)> DataCallback;
};

} // namespace Sound
} // namespace Leviathan
