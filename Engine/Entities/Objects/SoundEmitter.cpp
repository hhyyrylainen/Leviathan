#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SOUNDEMITTER
#include "SoundEmitter.h"
#endif
#include "Sound/SoundDevice.h"
#include "../../Handlers/IDFactory.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::SoundEmitter::SoundEmitter() : BaseObject(IDFactory::GetID(), NULL){

}

DLLEXPORT Leviathan::Entity::SoundEmitter::~SoundEmitter(){
	// make sure to stop sound //
	_LetGoOfPlayer();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::SoundEmitter::SetFileToPlay(const std::string &file,
    bool usestreaming /*= false*/)
{
	// we should probably let go of old one //
	_LetGoOfPlayer();

	// use SoundDevice to get a valid player //
	InternalSoundPlayer = SoundDevice::Get()->GetSlotForSound(file);

	// set our sound on it //
	InternalSoundPlayer->SetPlayFile(file, usestreaming);
}
// ------------------------------------ //
void Leviathan::Entity::SoundEmitter::PosUpdated(Lock &guard){
	if(InternalSoundPlayer){

		InternalSoundPlayer->SetPosition(Position);
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::SoundEmitter::Stop(){
	InternalSoundPlayer->Stop();
}

DLLEXPORT void Leviathan::Entity::SoundEmitter::Start(){
	InternalSoundPlayer->Play();
}
// ------------------------------------ //
void Leviathan::Entity::SoundEmitter::_LetGoOfPlayer(){
	if(InternalSoundPlayer){
		InternalSoundPlayer->Stop();
		InternalSoundPlayer->SetConnected(false);
	}
}




