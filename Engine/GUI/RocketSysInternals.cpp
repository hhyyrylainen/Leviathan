#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ROCKETSYSINTERNALS
#include "RocketSysInternals.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
Leviathan::RocketSysInternals::RocketSysInternals(){
	// get start time //
	TimeSinceStart = Misc::GetTimeMicro64();
}

Leviathan::RocketSysInternals::~RocketSysInternals(){

}
// ------------------------------------ //
float Leviathan::RocketSysInternals::GetElapsedTime(){
	return (Misc::GetTimeMicro64()-TimeSinceStart)/1000000.f;
}
// ------------------------------------ //
bool Leviathan::RocketSysInternals::LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String &message){

	wstring finalmsg = L"[ROCKET]";

	switch(type){
	case Rocket::Core::Log::LT_INFO:
	case Rocket::Core::Log::LT_MAX:
	default:
		finalmsg += L"[INFO] ";
	break;
	case Rocket::Core::Log::LT_DEBUG:
#ifndef _DEBUG
		return false;
#else
		finalmsg += L"[DEBUG] ";
	break;
#endif // _DEBUG
	case Rocket::Core::Log::LT_ALWAYS:
	case Rocket::Core::Log::LT_ASSERT:
	case Rocket::Core::Log::LT_ERROR:
		finalmsg += L"[ERROR] ";
	break;
	case Rocket::Core::Log::LT_WARNING:
		finalmsg += L"[WARNING] ";
	break;
	}

	finalmsg += Convert::StringToWstring(message.CString());

	Logger::Get()->Write(finalmsg, true);

	return false;
}
// ------------------------------------ //



