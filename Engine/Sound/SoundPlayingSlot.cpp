// ------------------------------------ //
#include "SoundPlayingSlot.h"

#include "Define.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::SoundPlayingSlot::SoundPlayingSlot() :
    Music(NULL), Audio(NULL), SoundBuffer(NULL), UnusedTimeMS(0), Linked(false)
{

}

DLLEXPORT Leviathan::SoundPlayingSlot::~SoundPlayingSlot(){
	// delete all //
	SAFE_DELETE(Music);
	SAFE_DELETE(Audio);
	SAFE_DELETE(SoundBuffer);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SoundPlayingSlot::Play(){
	if(Music)
		Music->play();
	else
		Audio->play();
	// reset unused state to not get recycled right after stopping //
	UnusedTimeMS = 0;
}

DLLEXPORT void Leviathan::SoundPlayingSlot::Stop(){
	if(Music)
		Music->stop();
	else
		Audio->stop();
}

DLLEXPORT void Leviathan::SoundPlayingSlot::SetRepeat(bool repeat){
	if(Music)
		Music->setLoop(repeat);
	else
		Audio->setLoop(repeat);
}

DLLEXPORT bool Leviathan::SoundPlayingSlot::IsStopped(){
	if(Music)
		return Music->getStatus() == sf::Music::Paused;
	return Audio->getStatus() == sf::Sound::Paused;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SoundPlayingSlot::SetPlayFile(const std::string &file, bool streaming){
	// check has file changed //
	if(file == FileName)
		return;
	// store new file //
	FileName = file;

	if(streaming){
		// delete old //
		SAFE_DELETE(Audio);
		SAFE_DELETE(SoundBuffer);
		if(!Music){
			Music = new sf::Music();
		}

		Music->openFromFile(FileName);

	} else {
		// delete old //
		SAFE_DELETE(Music);
		if(!Audio){
			Audio = new sf::Sound();
		}
		if(!SoundBuffer){
			SoundBuffer = new sf::SoundBuffer();
		}

		// load buffer //
		SoundBuffer->loadFromFile(FileName);

		Audio->setBuffer(*SoundBuffer);
	}
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
	if(Music)
		Music->setRelativeToListener(set);
	else
		Audio->setRelativeToListener(set);
	// reset unused state to not get recycled right after stopping //
	UnusedTimeMS = 0;
}

DLLEXPORT void Leviathan::SoundPlayingSlot::SetPosition(const Float3 &pos){
	if(Music)
		Music->setPosition(pos.X, pos.Y, pos.Z);
	else
		Audio->setPosition(pos.X, pos.Y, pos.Z);
	// reset unused state to not get recycled right after stopping //
	UnusedTimeMS = 0;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SoundPlayingSlot::SetMinDistance(const float &distance){
	if(Music)
		Music->setMinDistance(distance);
	else
		Audio->setMinDistance(distance);
}

DLLEXPORT void Leviathan::SoundPlayingSlot::SetAttenuation(const float &attenuation){
	if(Music)
		Music->setAttenuation(attenuation);
	else
		Audio->setAttenuation(attenuation);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SoundPlayingSlot::IsConnected(){
	return Linked;
}

DLLEXPORT void Leviathan::SoundPlayingSlot::SetConnected(bool state){
	Linked = state;
}


