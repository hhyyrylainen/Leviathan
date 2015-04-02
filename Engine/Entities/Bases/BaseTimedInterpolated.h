#pragma once
#define LEVIATHAN_BASETIMEDINTERPOLATED
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

    //! \brief Classes that can smootly move between states
    //! \note The object needs to be locked before calling any methods
	class BaseTimedInterpolated{
	public:

        BaseTimedInterpolated();

        //! \brief Sets this entity's state to be between first and second
        //! \param progress The progress between the states. Between 0.f and 1.f, Note it may actually be over 1.f so
        //! check if the progress is over one and snap to second
        DLLEXPORT virtual bool SetStateToInterpolated(ObjectDeltaStateData* first, ObjectDeltaStateData* second,
            float progress) = 0;

        //! \brief Sets the current interpolation
        //! \note The object needs to be locked before this call
        //! \return True when the object was walid
        DLLEXPORT bool SetCurrentInterpolation(const ObjectInterpolation &interpolation);

    protected:

        //! \brief Updates state according to the time and current animation
        void UpdateInterpolation(int mspassed);

        //! \brief Called when the current animation ends
        //! \return True when new animation is set and should begin immediately
        //! \note Won't be called when destructed with a still-playing animation
        //! 
        //! Prefer calling SetCurrentInterpolation to set the new interpolation to reset TimeAccumulator properly
        virtual bool OnInterpolationFinished();

        // ------------------------------------ //

        //! Holds the currently playing interpolation
        //! When CurrentAnimation.Duration == 0 not playing anything
        ObjectInterpolation CurrentAnimation;
        int TimeAccumulator;
	};

}
