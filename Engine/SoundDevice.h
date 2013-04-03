#ifndef LEVIATHAN_SOUND
#define LEVIATHAN_SOUND
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Logger.h"
#include "FileSystem.h"
#include <XAudio2.h>
#include <XAudio2fx.h>


#include "SoundStreamThread.h"
#include "FileHelper.h"

#define SOUNDTICKRATE 15


namespace Leviathan{
	struct PlayingSound{
	public:
		PlayingSound(){
			pVoice = NULL;
			ID = -1;
			Volume = 100;
			RepeatCount = 0;
			ended = true;
			Ending = true;

			pDataBuff = NULL;
			//wfx = WAVEFORMATEXTENSIBLE();
			buffer = XAUDIO2_BUFFER();

			File = L"";
		};

		PlayingSound(int id, wstring file, int volume, int repeatcount){
			pVoice = NULL;
			ID = id;
			Volume = volume;
			RepeatCount = repeatcount;
			ended = false;
			Ending = false;

			pDataBuff = NULL;
			//wfx = WAVEFORMATEXTENSIBLE();
			buffer = XAUDIO2_BUFFER();

			File = file;

		}
		// --------------- //
		IXAudio2SourceVoice* pVoice;
		int ID;
		int Volume;
		bool changed;
		int RepeatCount;

		bool Ending;
		bool ended;

		wstring File;
		BYTE* pDataBuff;

		WAVEFORMATEXTENSIBLE wfx;
		XAUDIO2_BUFFER buffer;
	};


	class SoundDevice : public EngineComponent{
	public:
		DLLEXPORT SoundDevice::SoundDevice();
        DLLEXPORT SoundDevice::~SoundDevice();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT void Tick(int PassedMs);

		DLLEXPORT bool StartPlaying(wstring file, int repeatcount, int volumepercent, int ID);
		DLLEXPORT void StopSound(int ID);
		DLLEXPORT void StopAll();

		DLLEXPORT void UpdateSound(int ID, int repeatcount, int volume);


		DLLEXPORT static IXAudio2* GetAudio();//{ return Instance->Device; };
		DLLEXPORT static HANDLE GetAbort();//{ return Instance->g_hAbortEvent; };


	private:
		static SoundDevice* Instance;
		// ------------------- //
		bool InitSoundStream(int index);
		void EndSoundStream(int index, bool WaitForthread);
		
		bool SoundPlay(int index);
		void EndSound(int index);

		bool InitSound();


		// -------------------- //
		vector<StreamContext*> Streams;
		vector<PlayingSound*> Sounds;

		IXAudio2* Device;
		IXAudio2MasteringVoice* MasterSound;
		//IXAudio2SourceVoice* pSourceVoice;

		// thread close event
		HANDLE g_hAbortEvent;

		// thread specific stuff //
		//StreamContext streamcontext;
		//DWORD LastID;
		//HANDLE hstreamingVoiceThread;

		int MsPassed;

	};

}
#endif