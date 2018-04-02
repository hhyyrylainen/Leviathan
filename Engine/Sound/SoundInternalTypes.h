// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "ProceduralSound.h"

#include "cAudio/IAudioDecoder.h"
#include "cAudio/IAudioDecoderFactory.h"
#include "cAudio/IDataSourceFactory.h"


namespace Leviathan { namespace Sound {

//! \brief Used to feed data retrieved from a callback to cAudio::IAudioSource
class ProceduralSoundStream : public cAudio::IAudioDecoder {
public:
    DLLEXPORT ProceduralSoundStream(ProceduralSoundData::pointer data);
    DLLEXPORT ~ProceduralSoundStream();

    // cAudio interface //

    DLLEXPORT cAudio::AudioFormats getFormat() override;

    DLLEXPORT int getFrequency() override;

    DLLEXPORT bool isSeekingSupported() override;

    DLLEXPORT bool isValid() override;

    DLLEXPORT int readAudioData(void* output, int amount) override;

    DLLEXPORT bool setPosition(int position, bool relative) override;

    DLLEXPORT bool seek(float seconds, bool relative) override;

    DLLEXPORT float getTotalTime() override;

    DLLEXPORT int getTotalSize() override;

    DLLEXPORT int getCompressedSize() override;

    DLLEXPORT float getCurrentTime() override;

    DLLEXPORT int getCurrentPosition() override;

    DLLEXPORT int getCurrentCompressedPosition() override;

    DLLEXPORT cAudio::cAudioString getType() const override;

protected:
    //! \brief Called when the stream should close or if the data source has been closed
    DLLEXPORT void onStreamEnded();

private:
    //! This is where the audio data is retrieved when streaming
    ProceduralSoundData::pointer Data;
};



// //! Factory class for ProceduralSoundSource converting to ProceduralSoundStream
// class ProceduralSoundStreamFactory : public cAudio::IAudioDecoderFactory {
// public:
//     //! Called by cAudio when the user of ProceduralSoundData opens the sound stream
//     returned
//     //! from SoundDevice
//     cAudio::IAudioDecoder* CreateAudioDecoder(cAudio::IDataSource* stream) override;
// };

// class ProceduralSoundSource : public cAudio::IDataSource {
// public:
//     ProceduralSoundSource(ProceduralSoundData::pointer data) : Data(data) {}

//     DLLEXPORT bool isValid() override
//     {
//         return Data != nullptr && Data->IsValid();
//     }

//     DLLEXPORT int getCurrentPos() override
//     {
//         return 0;
//     }

//     DLLEXPORT int getSize() override
//     {
//         return 0;
//     }

//     DLLEXPORT int read(void* output, int size) override
//     {
//         (void)output;
//         (void)size;
//         return 0;
//     }

//     DLLEXPORT bool seek(int amount, bool relative) override
//     {
//         (void)amount;
//         (void)relative;
//         return false;
//     }

//     ProceduralSoundData::pointer Data;
// };

// //! Factory class for ProceduralSoundStream
// class ProceduralSoundSourceFactory : public cAudio::IDataSourceFactory, public ThreadSafe {
// public:
//     DLLEXPORT ProceduralSoundSourceFactory();
//     DLLEXPORT ~ProceduralSoundSourceFactory();


//     DLLEXPORT cAudio::IDataSource* CreateDataSource(
//         const char* filename, bool streamingRequested) override;

//     //! \brief Reserves a stream name for ProceduralSoundData instance
//     DLLEXPORT void ReserveStream(
//         const std::string& fakefilename, ProceduralSoundData::pointer data);


//     //! \brief Removes a reserved stream, blocking CreateAudioDecoder from using the
//     //! same data twice. Called from ProceduralSoundStreamFactory when it creates one to not
//     //! allow duplicates
//     //!
//     //! Can also be called if the sound source needs to be destroyed before the creation is
//     //! finalized
//     DLLEXPORT void UnReserveStream(ProceduralSoundData::pointer data);

// private:
//     //! Contains streams that can be returned by CreateAudioDecoder to cAudio
//     //! \note Must be locked when changing this
//     std::map<std::string, ProceduralSoundData::pointer> m_OpenStream;
// };

}} // namespace Leviathan::Sound
