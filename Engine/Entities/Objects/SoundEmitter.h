#ifndef LEVIATHAN_SOUNDEMITTER
#define LEVIATHAN_SOUNDEMITTER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Bases/BaseObject.h"
#include "Entities/Bases/BasePositionable.h"
#include "Sound/SoundPlayingSlot.h"


namespace Leviathan{ namespace Entity{

	class SoundEmitter : public BaseObject, public BasePositionable{
	public:
		DLLEXPORT SoundEmitter();
		DLLEXPORT ~SoundEmitter();

		// NOTE: important to set usestreaming for long files to not freeze the program //
		DLLEXPORT void SetFileToPlay(const wstring &file, bool usestreaming = false);

		// \todo implement stream playing //

		// Audio controls //
		DLLEXPORT void Stop();
		DLLEXPORT void Start();

	protected:

		void _LetGoOfPlayer();
		// used to reposition the audio source //
		virtual void PosUpdated();

		// ------------------------------------ //
		shared_ptr<SoundPlayingSlot> InternalSoundPlayer;



	};

}}
#endif