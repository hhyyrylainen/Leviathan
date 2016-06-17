// ------------------------------------ //
#include "SoundPlayingSlot.h"

#include "cAudio.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::SoundPlayingSlot::SoundPlayingSlot() 
{

}

DLLEXPORT Leviathan::SoundPlayingSlot::~SoundPlayingSlot(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::SoundPlayingSlot::Play(){

    DEBUG_BREAK;
    
	// reset unused state to not get recycled right after stopping //
	UnusedTimeMS = 0;
}

DLLEXPORT void Leviathan::SoundPlayingSlot::Stop(){

}

DLLEXPORT void Leviathan::SoundPlayingSlot::SetRepeat(bool repeat){

}

DLLEXPORT bool Leviathan::SoundPlayingSlot::IsStopped(){

    DEBUG_BREAK;
    return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SoundPlayingSlot::SetPlayFile(const std::string &file,
    bool streaming)
{
	// check has file changed //
	if(file == FileName)
		return;
	// store new file //
	FileName = file;

    DEBUG_BREAK;
    
	// reset unused state to not get recycled right after stopping //
	UnusedTimeMS = 0;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SoundPlayingSlot::PassTimeIfNotPlaying(int mspassed){
	if(IsStopped()){

		UnusedTimeMS += mspassed;
	}
}

DLLEXPORT int Leviathan::SoundPlayingSlot::GetUnusedTime(){
	return UnusedTimeMS;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SoundPlayingSlot::SetRelativeToListener(bool set){
    DEBUG_BREAK;
    
	// reset unused state to not get recycled right after stopping //
	UnusedTimeMS = 0;
}

DLLEXPORT void Leviathan::SoundPlayingSlot::SetPosition(const Float3 &pos){

    DEBUG_BREAK;
    
	// reset unused state to not get recycled right after stopping //
	UnusedTimeMS = 0;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SoundPlayingSlot::SetMinDistance(const float &distance){

}

DLLEXPORT void Leviathan::SoundPlayingSlot::SetAttenuation(const float &attenuation){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SoundPlayingSlot::IsConnected(){
	return Linked;
}

DLLEXPORT void Leviathan::SoundPlayingSlot::SetConnected(bool state){
	Linked = state;
}


