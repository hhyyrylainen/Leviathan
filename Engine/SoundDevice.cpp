#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SOUND
#include "SoundDevice.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "WaveInfo.h"

SoundDevice* SoundDevice::Instance = NULL;


SoundDevice::SoundDevice(){
	// init pointers to null //
	Device = NULL;
	MasterSound = NULL;

	// set instance //
	Instance = this;

	//LastID = 11000;
	MsPassed = 0;
}
SoundDevice::~SoundDevice(){

}
// ------------------------------------ //
bool SoundDevice::Init(){

	// init main device //
	if(!this->InitSound()){

		Logger::Get()->Error(L"Failed to init SoundDevice, init XAudio2 failed", true);
		return false;
	}

	// load test sound //
	//this->SoundPlay(FileSystem::GetSoundFolder()+L"Survivor Eye Of The Tiger.wav");
	//StartPlaying(FileSystem::GetSoundFolder()+L"tsumtsum.wav", 20, 80, IDFactory::GetID());
	//StartPlaying(FileSystem::GetSoundFolder()+L"Survivor Eye Of The Tiger.wav", 1, 110, IDFactory::GetID());
	//StartPlaying(FileSystem::GetSoundFolder()+L"sound01.wav", 4, 56, IDFactory::GetID());

	return true;
}
void SoundDevice::Release(){
	// end streaming threads //


	SetEvent(g_hAbortEvent);

	// wait for threads to end //
	for(unsigned int i = 0;i < Streams.size(); i++){

		WaitForSingleObject(Streams.at(i)->hstreamingVoiceThread, INFINITE);
		// close handle for thread //
		CloseHandle(Streams.at(i)->hstreamingVoiceThread);
		delete Streams.at(i);
	}

	// close handles //
	Streams.clear();

	// close sounds //
	for(unsigned int i = 0; i < Sounds.size(); i++){
		// stop playing //
		Sounds.at(i)->pVoice->Stop();
		Sounds.at(i)->pVoice->FlushSourceBuffers();
		Sounds.at(i)->pVoice->DestroyVoice();

		SAFE_DELETE_ARRAY(Sounds.at(i)->pDataBuff);

		delete Sounds.at(i);

	}

	Sounds.clear();

	CloseHandle(g_hAbortEvent);

	// release engine //

	SAFE_RELEASE(Device);
	//SAFE_DELETE_ARRAY(pDataBuffer);
	
}
// ------------------------------ //
 void SoundDevice::Tick(int PassedMs){
	 MsPassed += PassedMs;

	 if(MsPassed < SOUNDTICKRATE){
		 // doesn't need updating //
		 return;
	 }
	 MsPassed = 0;

	// update sounds //

	for(unsigned int i = 0; i < Streams.size(); i++){
		if(Streams[i]->ended){
			WaitForSingleObject(Streams.at(i)->hstreamingVoiceThread, 25);
			// close handle for thread //
			CloseHandle(Streams.at(i)->hstreamingVoiceThread);
			delete Streams.at(i);

			Streams.erase(Streams.begin()+i);
			i--;
		}

	}

	XAUDIO2_VOICE_STATE state;

	for(unsigned int i = 0; i < Sounds.size(); i++){
		Sounds[i]->pVoice->GetState(&state);
		if(state.BuffersQueued == 0){ // buffers finished //
			Sounds.at(i)->RepeatCount--;
			if(Sounds.at(i)->RepeatCount < 1){

				// stop playing //
				Sounds.at(i)->pVoice->Stop();
				Sounds.at(i)->pVoice->FlushSourceBuffers();
				Sounds.at(i)->pVoice->DestroyVoice();

				SAFE_DELETE_ARRAY(Sounds.at(i)->pDataBuff);

				delete Sounds.at(i);
				Sounds.erase(Sounds.begin()+i);
				i--;
			} else {
				// replay buffer //
				Sounds[i]->pVoice->SubmitSourceBuffer(&Sounds.at(i)->buffer);
				Sounds[i]->pVoice->Start(0);
			}
		}
	}
	return;

 }
// ------------------------------ //
bool SoundDevice::InitSound(){
	HRESULT hr = S_OK;
	hr = XAudio2Create( &Device, NULL, XAUDIO2_DEFAULT_PROCESSOR);
	if(FAILED(hr)){

		Logger::Get()->Error(L"Failed to init XAudio2, XAudio2create failed", false);
		return false;
	}

	// create master voice //
	hr = Device->CreateMasteringVoice(&MasterSound);
	if(FAILED(hr)){

		Logger::Get()->Error(L"Failed to init XAudio2, CreateMasteringVoice failed", false);
		return false;
	}

	// create end message
	g_hAbortEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	return true;
}
// ------------------------------------ //
bool SoundDevice::StartPlaying(wstring file, int repeatcount, int volumepercent, int ID){
	
	if(!FileSystem::FileExists(file)){

		Logger::Get()->Error(L"SoundDevice startplaying fail: file doesn't exist!");
		return false;
	}

	bool Toolong = false;

	// check is file over 400 kt big //
	if(FileSystem::GetFileLength(file) > 400000)
		Toolong = true;

	if(Toolong){
		// start sound stream //
		Streams.push_back(new StreamContext(NULL, file, CreateEvent(NULL, FALSE, FALSE, NULL), CreateEvent(NULL, FALSE, FALSE, NULL)));
		Streams.back()->ID = ID;
		Streams.back()->Volume = volumepercent;
		Streams.back()->Repeatcount = repeatcount;

		// start //
		if(!InitSoundStream(Streams.size()-1)){
			EndSoundStream(Streams.size()-1, false);
			// set as invalid //
			//
			return false;
		}

	} else {
		// make constant buffer //
		Sounds.push_back(new PlayingSound(ID, file, volumepercent, repeatcount));

		if(!SoundPlay(Sounds.size()-1)){
			EndSound(Sounds.size()-1);
			Sounds.pop_back();
			return false;
		}
	}

	return true;
}
void SoundDevice::StopSound(int ID){
	// look for id //
	bool IsStream = false;
	int Index = -1;

	for(unsigned int i = 0; i < Streams.size(); i++){
		if(Streams[i]->ID == ID){
			IsStream = true;
			Index = i;
			break;
		}
	}

	if(!IsStream){
		for(unsigned int i = 0; i < Sounds.size(); i++){
			if(Sounds[i]->ID == ID){
				Index = i;
				break;
			}
		}
		if(Index == -1){
			Logger::Get()->Error(L"Sound with ID not found!"+ID, false);
			return;
		}

		// stop playing //
		Sounds.at(Index)->pVoice->Stop();
		Sounds.at(Index)->pVoice->FlushSourceBuffers();
		Sounds.at(Index)->pVoice->DestroyVoice();

		SAFE_DELETE_ARRAY(Sounds.at(Index)->pDataBuff);

		delete Sounds.at(Index);

		Sounds.erase(Sounds.begin()+Index);

	} else {
		if(Index == -1){
			Logger::Get()->Error(L"Sound with ID not found!"+ID, false);
			return;
		}

		Streams[Index]->End = true;
		// signal to update state //
		SetEvent(Streams.at(Index)->UpDateEvent);

	}


}
void SoundDevice::StopAll(){
	// set as ending //
	for(unsigned int i = 0;i < Streams.size(); i++){

		Streams[i]->End = true;
		// signal to update state //
		SetEvent(Streams.at(i)->UpDateEvent);
	}

	// close sounds //
	for(unsigned int i = 0; i < Sounds.size(); i++){
		// stop playing //
		Sounds.at(i)->pVoice->Stop();
		Sounds.at(i)->pVoice->FlushSourceBuffers();
		Sounds.at(i)->pVoice->DestroyVoice();

		SAFE_DELETE_ARRAY(Sounds.at(i)->pDataBuff);

		delete Sounds.at(i);
	}
	Sounds.clear();

}
void SoundDevice::UpdateSound(int ID, int repeatcount, int volume){
	// look for id //
	bool IsStream = false;
	int Index = -1;

	for(unsigned int i = 0; i < Streams.size(); i++){
		if(Streams[i]->ID == ID){
			IsStream = true;
			Index = i;
			break;
		}
	}

	if(!IsStream){
		for(unsigned int i = 0; i < Sounds.size(); i++){
			if(Sounds[i]->ID == ID){
				Index = i;
				break;
			}
		}
		if(Index == -1){
			Logger::Get()->Error(L"Sound with ID not found!"+ID, false);
			return;
		}

		// update vars //
		Sounds[Index]->Volume = volume;
		Sounds[Index]->RepeatCount = repeatcount;

	} else {
		if(Index == -1){
			Logger::Get()->Error(L"Sound with ID not found!"+ID, false);
			return;
		}

		// update //
		Streams[Index]->Volume = volume;
		Streams[Index]->Repeatcount = repeatcount;

		// signal to update state //
		SetEvent(Streams.at(Index)->UpDateEvent);

	}
}

// ------------------------------------ //
bool SoundDevice::InitSoundStream(int index){

	StreamContext* pTemp = Streams.at(index);

	// prepare context //
	//*pTemp = StreamContext(NULL, file.c_str(), CreateEvent(NULL, FALSE, FALSE, NULL));

	// create streaming thread //
	pTemp->ThreadID = 0;
	//LastID++;

	pTemp->hstreamingVoiceThread = CreateThread( NULL, 0, StreamProc, pTemp, 0, &pTemp->ThreadID );

	if(pTemp->hstreamingVoiceThread == NULL){
		// failed to create thread //
		Logger::Get()->Error(L"PlaySound: failed to create new thread", GetLastError(), true);
		return false;
	}

	// wait for the sound to load //
	WaitForSingleObject(pTemp->hVoiceLoadEvent, INFINITE); // don't wait more than 5 ms

	// check if succeeded //
	if((pTemp->pVoice == NULL)){
		// failed to load
		Logger::Get()->Error(L"PlaySound: thread failed to load sound file", GetLastError(), true);
		return false;
	}

	// start playing //
	pTemp->pVoice->Start();
	return true;
}
void SoundDevice::EndSoundStream(int index, bool WaitForthread){
	// set as ending //
	Streams.at(index)->End = true;
	// signal to update state //
	SetEvent(Streams.at(index)->UpDateEvent);

	if(WaitForthread){
		WaitForSingleObject(Streams.at(index)->hstreamingVoiceThread, INFINITE);
	}
}
bool SoundDevice::SoundPlay(int index){
	PlayingSound* pTemp = Sounds.at(index);

	// helper class by jay temnant //
	WaveInfo waveloader;

	waveloader.load(pTemp->File.c_str());


	// load buffer //
	HANDLE hFile = CreateFile(pTemp->File.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if(hFile == INVALID_HANDLE_VALUE){
		Logger::Get()->Error(L"SoundPlay: CreateFile failed", GetLastError(), false);
		return false;
	}

	if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER){
		Logger::Get()->Error(L"SoundPlay: set file pointer invalid", GetLastError(), false);
		return false;
	}

	DWORD length = waveloader.getDataLength();
	DWORD offset = waveloader.getDataOffset();

	pTemp->pDataBuff = new BYTE[length];

	if(FAILED(ReadChunkData(hFile, pTemp->pDataBuff, length, offset))){
		Logger::Get()->Error(L"SoundPlay: ReadChunkData failed", GetLastError(), false);
		return false;
	}

	// populate sound buffer //
	pTemp->buffer.AudioBytes = length;
	pTemp->buffer.pAudioData = pTemp->pDataBuff;
	pTemp->buffer.Flags = XAUDIO2_END_OF_STREAM;

	// create source voice //
	HRESULT hr = S_OK;

	hr = Device->CreateSourceVoice(&pTemp->pVoice, waveloader.wf());
	if(FAILED(hr)){
		Logger::Get()->Error(L"SoundPlay: Create source voice failed", hr, false);
		return false;
	}

	// submit buffer //
	hr = pTemp->pVoice->SubmitSourceBuffer(&pTemp->buffer); // buffer needs to stay allocated while sound plays
	if(FAILED(hr)){
		Logger::Get()->Error(L"SoundPlay: submit buffer failed", hr, false);
		return false;
	}

	pTemp->pVoice->SetVolume(((float)pTemp->Volume)/100);

	hr = pTemp->pVoice->Start(0);
	if(FAILED(hr)){
		Logger::Get()->Error(L"SoundPlay: submit buffer failed", hr, false);
		return false;
	}

	return true;
}
void SoundDevice::EndSound(int index){
	Sounds.at(index)->Ending = true;

	// stop playing //
	Sounds.at(index)->pVoice->Stop();
	Sounds.at(index)->pVoice->FlushSourceBuffers();
	Sounds.at(index)->pVoice->DestroyVoice();

	SAFE_DELETE_ARRAY(Sounds.at(index)->pDataBuff);

	delete Sounds.at(index);

}
// ------------------------------------ //
IXAudio2* SoundDevice::GetAudio(){ 
	return Instance->Device; 
}
HANDLE SoundDevice::GetAbort(){ 
	return Instance->g_hAbortEvent; 
}
// ------------------------------------ //