#pragma once
#ifndef LEVIATHAN_LEAPLISTENER
#define LEVIATHAN_LEAPLISTENER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Leap.h"

namespace Leviathan{

	class LeapListener : public Leap::Listener{
        friend LeapManager;
	public:
		LeapListener(LeapManager* owner);
		~LeapListener();

		// leap listener's virtual methods //
		virtual void onInit(const Leap::Controller &control);
		virtual void onConnect(const Leap::Controller &control);
		virtual void onDisconnect(const Leap::Controller &control);
		virtual void onExit(const Leap::Controller &control);
		virtual void onFrame(const Leap::Controller &control);
		virtual void onFocusGained(const Leap::Controller &control);
		virtual void onFocusLost(const Leap::Controller &control);

		DLLEXPORT inline bool IsConnected(){
			return Connected;
		}

    protected:

        //! \brief Handles a frame retrieved from the Leap or notified by frame event
        void HandleFrame(const Leap::Frame &frame);

	private:
		// access to owner to update states //
		LeapManager* Owner;

		// flags //
		bool Connected;
		bool Focused;
		// states //
	};

}
#endif
