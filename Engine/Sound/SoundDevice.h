#ifndef LEVIATHAN_SOUND
#define LEVIATHAN_SOUND
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SoundPlayingSlot.h"


#define SOUND_UNLOAD_UNUSEDTIME		30000
#define MAX_CONCURRENT_SOUNDS		256

namespace Leviathan{


	class SoundDevice : public EngineComponent{
	public:
		DLLEXPORT SoundDevice::SoundDevice();
		DLLEXPORT SoundDevice::~SoundDevice();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT void Tick(int PassedMs);

		DLLEXPORT void SetSoundListenerPosition(const Float3 &pos, const Float3 &pitchyawroll);
		DLLEXPORT void SetGlobalVolume(const float &vol);
		// Getting proper sound stream functions //

		DLLEXPORT shared_ptr<SoundPlayingSlot> GetSlotForSound(const wstring &file);
		// used for streams //
		DLLEXPORT shared_ptr<SoundPlayingSlot> GetSlotForSound();

		DLLEXPORT static inline SoundDevice* Get(){
			return Instance;
		}
	private:



		// ------------------------------------ //
		std::vector<shared_ptr<SoundPlayingSlot>> LoadedSoundObjects;


		static SoundDevice* Instance;
	};

}
#endif