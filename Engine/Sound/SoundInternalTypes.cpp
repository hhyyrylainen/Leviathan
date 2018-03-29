// ------------------------------------ //
#include "SoundInternalTypes.h"

using namespace Leviathan;
using namespace Leviathan::Sound;
// ------------------------------------ //

// cAudio::IAudioDecoder* ProceduralSoundStreamFactory::CreateAudioDecoder(
//     cAudio::IDataSource* stream)
// {
//     return CAUDIO_NEW ProceduralSoundStream(static_cast<ProceduralSoundSource*>(stream),
//         static_cast<ProceduralSoundSource*>(stream)->Data);
// }

// MemoryDataSourceFactory::MemoryDataSourceFactory() {}

// MemoryDataSourceFactory::~MemoryDataSourceFactory() {}

// cAudio::IDataSource* MemoryDataSourceFactory::CreateDataSource(
//     const char* filename, bool streamingRequested)
// {
//     if(!streamingRequested) {

//         LOG_FATAL("streaming NOT requested for a sound memory stream");
//         return nullptr;
//     }

//     GUARD_LOCK();

//     auto iter = m_OpenStream.find(std::string(filename));

//     if(iter == m_OpenStream.end()) {

//         // This occurs during normal use, as any sound created will try to open
//         // a video audio stream, so just silently fail and everything will be fine
//         return nullptr;
//     }

//     ProceduralSoundData::pointer data = iter->second;

//     m_OpenStream.erase(iter);

//     return CAUDIO_NEW VideoPlayerSource(data);
// }

// void MemoryDataSourceFactory::reserveStream(
//     const std::string& fakeFileName, VideoPlayer* streamSource)
// {
//     boost::lock_guard<boost::mutex> lock(m_Mutex);

//     m_OpenStream[fakeFileName] = streamSource;
// }

// void MemoryDataSourceFactory::unReserveStream(VideoPlayer* streamSource)
// {
//     boost::lock_guard<boost::mutex> lock(m_Mutex);

//     for(auto iter = m_OpenStream.begin(); iter != m_OpenStream.end(); ++iter) {

//         if(iter->second == streamSource) {

//             m_OpenStream.erase(iter);
//             return;
//         }
//     }
// }


// ------------------------------------ //
// ProceduralSoundStream
DLLEXPORT ProceduralSoundStream::ProceduralSoundStream(ProceduralSoundData::pointer data) :
    IAudioDecoder(nullptr), Data(data)
{
    LEVIATHAN_ASSERT(Data, "ProceduralSoundStream created without data source");

    // if(Data)
    //     Data->StreamReportingIn(this);
}

DLLEXPORT ProceduralSoundStream::~ProceduralSoundStream() {}

// cAudio interface //
DLLEXPORT cAudio::AudioFormats ProceduralSoundStream::getFormat()
{
    return Data->Properties.Format;
}

DLLEXPORT int ProceduralSoundStream::getFrequency()
{
    return Data->Properties.SampleRate;
}

DLLEXPORT bool ProceduralSoundStream::isSeekingSupported()
{
    return false;
}

DLLEXPORT bool ProceduralSoundStream::isValid()
{
    return Data->IsValid();
}

DLLEXPORT int ProceduralSoundStream::readAudioData(void* output, int amount)
{
    return Data->ReadAudioData(output, amount);
}

DLLEXPORT bool ProceduralSoundStream::setPosition(int position, bool relative)
{
    (void)position;
    (void)relative;
    return false;
}

DLLEXPORT bool ProceduralSoundStream::seek(float seconds, bool relative)
{
    (void)seconds;
    (void)relative;
    return false;
}

DLLEXPORT float ProceduralSoundStream::getTotalTime()
{
    return -1.f;
}

DLLEXPORT int ProceduralSoundStream::getTotalSize()
{
    return -1;
}

DLLEXPORT int ProceduralSoundStream::getCompressedSize()
{
    return -1;
}

DLLEXPORT float ProceduralSoundStream::getCurrentTime()
{
    return -1.f;
}

DLLEXPORT int ProceduralSoundStream::getCurrentPosition()
{
    return -1;
}

DLLEXPORT int ProceduralSoundStream::getCurrentCompressedPosition()
{
    return -1;
}

DLLEXPORT cAudio::cAudioString ProceduralSoundStream::getType() const
{
#ifdef _WIN32
	return L"Leviathan::ProceduralSoundStream";
#else
    return "Leviathan::ProceduralSoundStream";
#endif
}

DLLEXPORT void ProceduralSoundStream::onStreamEnded()
{
    // Data->StreamReportingIn(nullptr);
}
