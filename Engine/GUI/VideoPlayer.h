// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Events/CallableObject.h"
#include "Events/DelegateSlot.h"
#include "Sound/SoundDevice.h"
#include "TimeIncludes.h"

#include "bsfCore/BsCorePrerequisites.h"

#include <chrono>
#include <vector>


namespace Leviathan { namespace GUI {

//! \brief VideoPlayer that uses AOM to play videos on a texture
//!
//! Supports playing av1 video streams with either vorbis or opus audio
//! \todo Implement pausing and seeking
//! \todo When Stop is called the OnPlaybackEnded should still be fired. If something was
//! playing
class VideoPlayer : public CallableObject {
    struct Implementation;

public:
    DLLEXPORT VideoPlayer();
    DLLEXPORT ~VideoPlayer();

    VideoPlayer& operator=(const VideoPlayer&) = delete;

    //! \brief Starts playing the video file
    //! \returns True if successfully started
    DLLEXPORT bool Play(const std::string& videofile);

    //! \brief Stops playing and unloads the current playback objects
    DLLEXPORT void Stop();

    // ------------------------------------ //
    // Stream info

    //! \returns True if currently loaded file has an audio stream
    DLLEXPORT bool HasAudio() const
    {
        return HasAudioStream;
    }

    //! \returns Current playback position, in seconds
    //! The return value is directly read from the last decoded frame timestamp
    DLLEXPORT float GetCurrentTime() const;

    //! \returns The total length of the video is seconds. -1 if invalid
    DLLEXPORT float GetDuration() const;

    //! \returns Width of the current video
    DLLEXPORT int32_t GetVideoWidth() const
    {
        return FrameWidth;
    }

    //! \returns Height of the current video
    DLLEXPORT int32_t GetVideoHeight() const
    {
        return FrameHeight;
    }


    //! \returns The number of audio channels
    DLLEXPORT int GetAudioChannelCount() const
    {
        return ChannelCount;
    }

    //! \returns The number of samples per second of the audio stream
    //! or -1 if no audio streams exist
    DLLEXPORT int GetAudioSampleRate() const
    {
        return SampleRate;
    }

    //! \returns true if all the ffmpeg stream objects are valid for playback
    DLLEXPORT bool IsStreamValid() const
    {
        return StreamValid; // && VideoCodec && ConvertedFrameBuffer;
    }

    DLLEXPORT auto GetTexture() const
    {
        return VideoOutputTexture;
    }

public:
    //! \brief Reads audio data to the buffer
    //! \returns The number of sample frames read
    //! \param amount The maximum number of sample frames to read
    //! \protected
    size_t ReadAudioData(uint8_t* output, size_t amount);

protected:
    //! After loading the video this creates the output texture + material for it
    //! \returns false if the setup fails
    bool CreateOutputTexture();

    //! \brief Opens and parses the video info and opens decoding contexts
    //! \returns false if something fails
    bool OpenCodecsForFile();

    bool HandleFrameVideoUpdate();

    //! \brief Decodes one video frame. Returns false if more data is required
    //! but the stream ended (this condition ends the playback)
    bool DecodeVideoFrame();

    bool PeekNextFrameTimeStamp();

    //! \brief Updates the texture
    void UpdateTexture();

    //! \brief Reads already decoded audio data. The audio data vector must be locked
    //! before calling this
    size_t ReadDataFromAudioQueue(Lock& audiolocked, uint8_t* output, size_t amount);

    //! \brief Resets timers. Call when playback start or resumes
    void ResetClock();

    //! \brief Called when end of playback has been reached
    //!
    //! Closes the playback and invokes the delegates
    void OnStreamEndReached();

public:
    // CallableObject
    DLLEXPORT int OnEvent(Event* event) override;
    DLLEXPORT int OnGenericEvent(GenericEvent* event) override;


protected:
    std::unique_ptr<Implementation> Pimpl;

    std::string VideoFile;

    //! The target texture
    bs::HTexture VideoOutputTexture;

    //! True when playing back something and frame start events do something
    bool IsPlaying = false;

    //! How many timestamp units are in a second in the video stream
    double VideoTimeBase = 1.f;

    int32_t FrameWidth = 0;
    int32_t FrameHeight = 0;

    //! Audio sample rate
    int SampleRate = 0;
    int ChannelCount = 0;

    //! Used to start the audio playback once
    bool IsPlayingAudio = false;

    //! Set to true when an audio stream is found and opened
    bool HasAudioStream = false;

    //! Audio output
    AudioSource::pointer AudioStream;
    Sound::ProceduralSoundData::pointer AudioStreamData;
    Sound::ProceduralSoundData::SoundProperties AudioStreamDataProperties;

    // Timing control
    float PassedTimeSeconds = 0.f;

    //! Set to false if an error occurs and playback should stop
    std::atomic<bool> StreamValid{false};

public:
    //! Called when current video stops player
    //! \todo Should be renamed to OnPlaybackEnded
    Delegate OnPlayBackEnded;
};

}} // namespace Leviathan::GUI
