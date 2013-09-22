#ifndef LEVIATHAN_ROCKETSYSINTERNALS
#define LEVIATHAN_ROCKETSYSINTERNALS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <Rocket/Core/SystemInterface.h>

namespace Leviathan{

	class RocketSysInternals : public Rocket::Core::SystemInterface{
	public:
		RocketSysInternals();
		virtual ~RocketSysInternals();

		// time is in seconds //
		virtual float GetElapsedTime();

		// logs message //
		virtual bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String &message);

	private:

		__int64 TimeSinceStart;
	};

}
#endif