#pragma once
// ------------------------------------ //
#include "Include.h"
#include "Leap.h"

namespace Leviathan{

    class LeapManager;

	class LeapListener : public Leap::Listener{
        friend LeapManager;
	public:
		LeapListener(LeapManager* owner);
		~LeapListener();

		// leap listener's virtual methods //
        void onInit(const Leap::Controller &control) override;
        void onConnect(const Leap::Controller &control) override;
        void onDisconnect(const Leap::Controller &control) override;
        void onExit(const Leap::Controller &control) override;
        void onFrame(const Leap::Controller &control) override;
        void onFocusGained(const Leap::Controller &control) override;
        void onFocusLost(const Leap::Controller &control) override;

		DLLEXPORT inline bool IsConnected(){
			return Connected;
		}

    protected:

        //! \brief Handles a frame retrieved from the Leap or notified by frame event
        void HandleFrame(const Leap::Frame &frame, const Leap::Controller &control);

	private:
		// access to owner to update states //
		LeapManager* Owner;

		// flags //
		bool Connected;
		bool Focused;
		// states //

        //! Keeps track of handled frame count
        int HandledFrames;
	};

}

