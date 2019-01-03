// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Events/CallableObject.h"
#include "Events/DelegateSlot.h"
#include "Sound/SoundDevice.h"

#include "OgreTexture.h"

#include <chrono>
#include <vector>

extern "C" {
// FFMPEG includes
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

namespace Leviathan { namespace GUI {

//! \brief VideoPlayer that uses ffmpeg to play videos on Ogre
//!
//! textures and sound This is based on the VideoPlayer in Thrive
//! written by Henri Hyyryläinen which in turn was based on
//! ogre-ffmpeg-videoplayer, but all original ogre-ffmpeg-videoplayer
//! code has been removed in course of all the rewrites.
//! \todo Implement pausing and seeking
//! \todo When Stop is called the OnPlaybackEnded should still be fired. If something was
//! playing
//! \todo This should probably be in the rendering folder instead of the GUI folder
class VideoPlayer : public CallableObject {
protected:
    using ClockType = std::chrono::steady_clock;

    // These targets are attempted to be hit by the decoding
    constexpr static auto TARGET_BUFFERED_FRAMES = 5;
    constexpr static auto TARGET_BUFFERED_AUDIO = 60;

    //! \brief Holds converted audio data that was decoded while decoding video frames
    struct DecodedAudioData {

        std::vector<uint8_t> DecodedData;
        //! This is used by the audio thread if it can't read the whole thing at once
        //! in order to not need to free memory
        size_t CurrentReadOffset = 0;

        //! True once this has been played completely and can be recycled
        bool Played = true;
    };

    //! brief Holds converted video data that hasn't bee displayed yet
    struct DecodedVideoData {

        std::vector<uint8_t> FrameData;

        //! The time in seconds when this frame should be displayed
        float Timestamp = 0.f;

        //! Once played this is refilled
        bool Played = true;
    };

public:
    DLLEXPORT VideoPlayer();
    DLLEXPORT ~VideoPlayer();

    VideoPlayer& operator=(const VideoPlayer&) = delete;

    //! \brief Starts playing the video file
    //! \returns True if successfully started
    //! \note Acquires the Ogre texture, a sound player and ffmpeg resources
    DLLEXPORT bool Play(const std::string& videofile);

    //! \brief Stops playing and unloads the current playback objects
    DLLEXPORT void Stop();

    // ------------------------------------ //
    // Stream info

    //! \returns True if currently loaded file has an audio stream
    DLLEXPORT bool HasAudio() const
    {
        return AudioCodec != nullptr;
    }

    //! \returns Current playback position, in seconds
    //! \version This is now the actual time in seconds since the playback started and not the
    //! timestamp of the current or next frame
    DLLEXPORT float GetCurrentTime() const
    {
        return PassedTimeSeconds;
    }

    //! \returns The total length of the video is seconds
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
        return StreamValid && VideoCodec;
    }

    DLLEXPORT auto GetTextureName() const
    {
        return TextureName;
    }

    DLLEXPORT auto GetTexture() const
    {
        return VideoOutputTexture;
    }

    //! \brief Dumps info about loaded ffmpeg streams
    DLLEXPORT void DumpInfo() const;

    //! \brief Tries to call ffmpeg initialization once
    DLLEXPORT static void LoadFFMPEG();

public:
    //! \brief Reads audio data to the buffer
    //! \returns The number of bytes read
    //! \param amount The maximum number of bytes to read
    //! \protected
    size_t ReadAudioData(uint8_t* output, size_t amount);

protected:
    //! After loading the video this creates the output texture + material for it
    //! \returns false if the setup fails
    //! \todo Rename to CreateOutputTexture
    bool OnVideoDataLoaded();


    //! \brief Opens and parses the video info into ffmpeg streams and such
    //! \returns false if something fails
    bool FFMPEGLoadFile();

    //! helper for FFMPEGLoadFile
    //! \returns true on success
    bool OpenStream(unsigned int index, bool video);

    //! \brief Decodes one video frame. Returns false if more data is required
    //! by the decoder
    //!
    //! The decoded data is copied to DecodedVideoDataBuffer
    bool DecodeVideoFrame();

    //! \brief Decodes one audio frame. Returns false if more data is required
    //! by the decoder
    //!
    //! The decoded data is copied to DecodedAudioDataBuffer
    bool DecodeAudioFrame();

    //! \brief Reads data and decodes it from the video and audio streams until
    //! TARGET_BUFFERED_FRAMES and TARGET_BUFFERED_AUDIO are hit
    void FillPlaybackBuffers();

    //! \brief Updates the texture
    //!
    //! Also marks the frame as having been played
    void UpdateTexture(DecodedVideoData& frametoshow);

    //! \brief Called when end of playback has been reached
    //!
    //! Closes the playback and invokes the delegates
    void OnStreamEndReached();

    //! \returns The number of ready video frames
    int CountReadyFrames() const;

    //! \returns The number of ready audio "frames"
    //! \todo Could have a method for determining how many *seconds* worth of audio is ready
    int CountReadyAudioBuffers() const;

    //! Video streem seaking. Don't use as the audio will get out of sync
    DLLEXPORT void SeekVideo(float time);

public:
    // CallableObject
    DLLEXPORT int OnEvent(Event* event) override;
    DLLEXPORT int OnGenericEvent(GenericEvent* event) override;


protected:
    std::string VideoFile;

    // The target texture
    std::string TextureName;
    Ogre::TexturePtr VideoOutputTexture;

    //! True when playing back something and frame start events do something
    bool IsPlaying = false;

    // Could implement a custom file reader
    // AVIOContext* ResourceReader = nullptr;

    AVFormatContext* FormatContext = nullptr;

    AVCodecContext* VideoCodec = nullptr;
    int VideoIndex = 0;

    // AVCodecParserContext* AudioParser = nullptr;
    AVCodecContext* AudioCodec = nullptr;
    int AudioIndex = 0;


    SwsContext* ImageConverter = nullptr;

    SwrContext* AudioConverter = nullptr;

    AVFrame* DecodedFrame = nullptr;
    AVFrame* DecodedAudio = nullptr;

    //! Once a frame has been loaded to DecodedFrame it is converted
    //! to a format that Ogre texture can accept
    AVFrame* ConvertedFrame = nullptr;

    //! This is the backing buffer for ConvertedFrame
    //! \note This is not a smart pointer because this needs to be released with av_freep
    uint8_t* ConvertedFrameBuffer = nullptr;

    //! Required size for a single converted frame
    size_t ConvertedBufferSize = 0;

    //! Decoded video data. It is stored here rather than as ffmpeg packets because we need to
    //! read a pretty large buffer of audio and we might as well read video data as well
    std::vector<DecodedVideoData> DecodedVideoDataBuffer;


    //! How many timestamp units are in a second in the video stream
    float VideoTimeBase = 1.f;

    int32_t FrameWidth = 0;
    int32_t FrameHeight = 0;

    //! Audio sample rate
    int SampleRate = 0;
    int ChannelCount = 0;

    std::list<DecodedAudioData> DecodedAudioDataBuffer;
    Mutex AudioMutex;

    //! Used to start the audio playback once data has been decoded
    bool IsPlayingAudio = false;

    //! Audio output
    AudioSource::pointer AudioStream;
    ProceduralSoundData::pointer AudioStreamData;

    // Timing control
    float PassedTimeSeconds = 0.f;
    ClockType::time_point LastUpdateTime;

    //! Set to false if an error occurs and playback should stop
    std::atomic<bool> StreamValid{false};

    //! Set to true when the stream has ended. This is separate because with the data buffering
    //! there are still probably multiple frames to play
    bool ReadReachedEnd = false;

    //! True when the playback has started and no frames have been decoded yet
    //! Also if paused this will need to be set true when resuming
    bool FirstCallbackAfterPlay = true;

public:
    //! Called when current video stops player
    //! \todo Should be renamed to OnPlaybackEnded
    Delegate OnPlayBackEnded;

private:
    //! Sequence number for video textures
    static std::atomic<int> TextureSequenceNumber;
};

}} // namespace Leviathan::GUI
