#ifndef LEVIATHAN_SOUNDPLAYINGSLOT
#define LEVIATHAN_SOUNDPLAYINGSLOT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <SFML/Audio.hpp>

namespace Leviathan{



	class SoundPlayingSlot : public Object{
	public:
		DLLEXPORT SoundPlayingSlot();
		DLLEXPORT ~SoundPlayingSlot();

		// playback control //
		DLLEXPORT void Play();
		DLLEXPORT void Stop();
		// this is not cached in this class (set each time changing file - just to be sure) //
		DLLEXPORT void SetRepeat(bool repeat);
		DLLEXPORT bool IsStopped();

		// must be linked to one controller at a time //
		DLLEXPORT bool IsConnected();
		DLLEXPORT void SetConnected(bool state);

		DLLEXPORT void SetPlayFile(const wstring &file, bool streaming);
		// unused recycle control //
		DLLEXPORT void PassTimeIfNotPlaying(int mspassed);
		DLLEXPORT int GetUnusedTime();

		// sound control for pitch (etc.) //
		// TODO: do this
		// Note: none of these functions cache their state, so call them when starting playing //
		DLLEXPORT void SetRelativeToListener(bool set);
		DLLEXPORT void SetMinDistance(const float &distance);
		DLLEXPORT void SetAttenuation(const float &attenuation);

		// Note: use mono audio files when trying to position them //
		DLLEXPORT void SetPosition(const Float3 &pos);

	protected:
		// information about last played sound //
		wstring FileName;
		int UnusedTimeMS;
		bool Linked;

		// SFML sound playing interface //
		sf::Music* Music;
		sf::SoundBuffer* SoundBuffer;
		sf::Sound* Audio;

	};

}
#endif