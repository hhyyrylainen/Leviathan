#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SOUND
#include "SoundDevice.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

// might as well resize space for maximum number of playing sounds //
SoundDevice::SoundDevice() : LoadedSoundObjects(MAX_CONCURRENT_SOUNDS, shared_ptr<SoundPlayingSlot>(nullptr)){

	// set instance //
	Instance = this;

}
SoundDevice::~SoundDevice(){
	// reset instance and let smart pointers take care of rest //
	Instance = NULL;
}

SoundDevice* SoundDevice::Instance = NULL;
// ------------------------------------ //
bool SoundDevice::Init(){

	// setup global volume //
	SetGlobalVolume(50.f);

	return true;
}
void SoundDevice::Release(){
	LoadedSoundObjects.clear();
}
// ------------------------------ //
void SoundDevice::Tick(int PassedMs){
	// we can probably mark sounds that should be recycled in the future //
	bool delmore = true;

	for(size_t i = 0; i < LoadedSoundObjects.size(); i++){

		LoadedSoundObjects[i]->PassTimeIfNotPlaying(PassedMs);

		if(delmore){

			// very old sounds should be marked as re-use for something else //
			if(LoadedSoundObjects[i]->GetUnusedTime() >= SOUND_UNLOAD_UNUSEDTIME){
				// delete this sound //
				LoadedSoundObjects.erase(LoadedSoundObjects.begin()+i);
				// should be fine to stop here //
				delmore = false;
			}
		}
	}
}

DLLEXPORT shared_ptr<SoundPlayingSlot> Leviathan::SoundDevice::GetSlotForSound(const wstring &file){
	// loop all and get an empty one or if some already has it return it (if not active) //
	for(size_t i = 0; i < LoadedSoundObjects.size(); i++){
		if(LoadedSoundObjects[i]){
			// check is linked //
			if(!LoadedSoundObjects[i]->IsConnected()){
				// we can probably just return this one //
				// TODO: add find same file for efficiency //
				return LoadedSoundObjects[i];
			}

		} else {
			// add new //
			LoadedSoundObjects[i] = shared_ptr<SoundPlayingSlot>(new SoundPlayingSlot());
			return LoadedSoundObjects[i];
		}
	}
	// cannot find space for new one //
	// TODO: clear old ones here //

	return NULL;
}

DLLEXPORT shared_ptr<SoundPlayingSlot> Leviathan::SoundDevice::GetSlotForSound(){
	throw exception("not implemented");
}

DLLEXPORT void Leviathan::SoundDevice::SetSoundListenerPosition(const Float3 &pos, const Float3 &pitchyawroll){
	sf::Listener::setPosition(pos.X, pos.Y, pos.X);
	// we need to create a vector from the angles //

	Float3 vec = Float3(-sin(pitchyawroll.X*DEGREES_TO_RADIANS), sin(pitchyawroll.Y*DEGREES_TO_RADIANS), -cos(pitchyawroll.X*DEGREES_TO_RADIANS));

	sf::Listener::setDirection(vec.X, vec.Y, vec.Z);
}

DLLEXPORT void Leviathan::SoundDevice::SetGlobalVolume(const float &vol){
	sf::Listener::setGlobalVolume(vol);
}

// ------------------------------ //
