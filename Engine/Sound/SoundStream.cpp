// ------------------------------------ //
#include "SoundStream.h"


using namespace Leviathan;
// ------------------------------------ //
SoundStream::SoundStream(std::function<bool (std::vector<int16_t>&)> datacallback,
    unsigned int channelcount, unsigned int samplerate) :
    MoreDataCallback(datacallback)
{
    LEVIATHAN_ASSERT(MoreDataCallback, "Empty data callback passed to SoundStream");

    initialize(channelcount, samplerate);
}
// ------------------------------------ //
bool SoundStream::onGetData(Chunk& data){

    bool keepPlaying = MoreDataCallback(PlayingAudioData);

    if(!keepPlaying)
        return false;
    
    data.sampleCount = PlayingAudioData.size();
    data.samples = PlayingAudioData.data();
    return true;
}
