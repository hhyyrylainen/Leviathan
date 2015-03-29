#pragma once
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

//! Defines the networking mode
//! In this mode the server sends snapshots of moving entities to all clients which then interpolate
//! between states. Input will not be replicated on all clients. Clients locally simulate their own
//! inputs. Server uses resimulation to simulate clients taking actions in the past.
#define NETWORK_USE_SNAPSHOTS

#ifndef NETWORK_USE_SNAPSHOTS
//! In this mode all clients run the whole simulation with access to all inputs from all clients.
//! The server then sends verification snapshots to clients who then resimulate and interpolate if
//! their results were different.
#define NETWORK_USE_RESIMULATE
#endif //NETWORK_USE_SNAPSHOTS


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
	};
    
	// has no virtual destructor, objects may not be pointed by this base class //
	class EngineComponent : public Object{
	public:
		DLLEXPORT EngineComponent();

		DLLEXPORT virtual bool Init();
		DLLEXPORT virtual void Release();
	};
}

#include <boost/chrono/system_clocks.hpp>

// Standard type time durations //
typedef boost::chrono::duration<__int64, boost::milli> MillisecondDuration;
typedef boost::chrono::duration<__int64, boost::micro> MicrosecondDuration;
typedef boost::chrono::duration<float, boost::ratio<1>> SecondDuration;



#ifdef _WIN32
// This could also use the high_resolution_clock (because they both resolve to the same thing) //
//typedef boost::chrono::steady_clock WantedClockType;
typedef boost::chrono::high_resolution_clock WantedClockType;
#else

typedef boost::chrono::high_resolution_clock WantedClockType;

#endif

#include "Common/Types.h"
#include "Utility/Convert.h"

#include "Handlers/IDFactory.h"
#include "Logger.h"
#include "Statistics/TimingMonitor.h"


#endif
