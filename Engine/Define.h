#ifndef LEVIATHAN_DEFINE
#define LEVIATHAN_DEFINE
#ifndef LEVIATHAN_INCLUDE
#include "Include.h"
#endif

#include "ForwardDeclarations.h"
#include <boost/ratio.hpp>

//! Number of milliseconds between engine and world ticks
#define TICKSPEED 50

//! When true entities may run a single physical update with a short timestep when resimulating
//#define ALLOW_RESIMULATE_CONSUME_ALL

namespace Leviathan{

	template<class T>
	void SafeReleaser(T* obj){
		SAFE_RELEASE(obj);
	}
	template<class T>
	void SafeReleaseDeleter(T* obj){
		SAFE_RELEASEDEL(obj);
	}

	class Object{
	public:
		DLLEXPORT Object();;
		DLLEXPORT virtual ~Object();
		DLLEXPORT virtual bool IsThis(Object* compare);

	protected:

	};
	// has no virtual destructor, objects may not be pointed by this base class //
	class EngineComponent : public Object{
	public:
		DLLEXPORT EngineComponent();

		DLLEXPORT virtual bool Init();
		DLLEXPORT virtual void Release();
		DLLEXPORT inline bool IsInited(){
			return Inited;
		}
	protected:
		bool Inited;

	};
}

// Standard type time durations //
typedef boost::chrono::duration<__int64, boost::milli> MillisecondDuration;
typedef boost::chrono::duration<__int64, boost::micro> MicrosecondDuration;
typedef boost::chrono::duration<float, boost::ratio<1>> SecondDuration;

#include <boost/chrono/system_clocks.hpp>

#ifdef _WIN32
// This could also use the high_resolution_clock (because they both resolve to the same thing) //
//typedef boost::chrono::steady_clock WantedClockType;
typedef boost::chrono::high_resolution_clock WantedClockType;
#else

typedef boost::chrono::high_resolution_clock WantedClockType;

#endif

#include "Handlers/IDFactory.h"


#endif
