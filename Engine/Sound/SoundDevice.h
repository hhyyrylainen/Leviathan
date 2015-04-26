#pragma once
// ------------------------------------ //
#include "../Common/Types.h"
#include <memory>
#include <vector>
#include "SoundPlayingSlot.h"


#define SOUND_UNLOAD_UNUSEDTIME		30000
#define MAX_CONCURRENT_SOUNDS		256

namespace Leviathan{

	class SoundDevice{
	public:
		DLLEXPORT SoundDevice();
		DLLEXPORT ~SoundDevice();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT void Tick(int PassedMs);

		DLLEXPORT void SetSoundListenerPosition(const Float3 &pos, const Float3 &pitchyawroll);
		DLLEXPORT void SetGlobalVolume(const float &vol);
		// Getting proper sound stream functions //

		DLLEXPORT std::shared_ptr<SoundPlayingSlot> GetSlotForSound(const std::string &file);
		// used for streams //
		DLLEXPORT std::shared_ptr<SoundPlayingSlot> GetSlotForSound();

		DLLEXPORT static inline SoundDevice* Get(){
			return Instance;
		}
	private:



		// ------------------------------------ //
		std::vector<std::shared_ptr<SoundPlayingSlot>> LoadedSoundObjects;


		static SoundDevice* Instance;
	};

}

