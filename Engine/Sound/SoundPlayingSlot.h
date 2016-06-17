// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"

#include "../Common/Types.h"
#include <memory>
#include <vector>

namespace Leviathan{

    //! \todo Redo with cAudio
	class SoundPlayingSlot{
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

		DLLEXPORT void SetPlayFile(const std::string &file, bool streaming);
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
		std::string FileName;
		int UnusedTimeMS = 0;
		bool Linked = false;


	};

}

