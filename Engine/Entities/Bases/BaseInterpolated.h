#pragma once
#ifndef LEVIATHAN_BASEINTERPOLATED
#define LEVIATHAN_BASEINTERPOLATED
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseRenderable.h"
#include "Events/CallableObject.h"

//! Number of frames in which the actual position will be reached
#define INTERPOLATION_TARGET_REACH_IN_FRAMES 8

//! Required precision to stop interpolating
#define INTERPOLATION_STOP_PRECISION 0.04f

namespace Leviathan{


    //! \brief Makes entities move more smoothly
    //!
    //! Used to reduce jitter on the client when entities are resimulated
    //! \note CallableObject is privately inherited as this class needs access to everything CallbableObject
    //! exposes and no child class should be able to mess with them. (This could also be solved by a member
    //! of type CallableObject)
    //! \todo Make objects that don't do any interpolation not consume as much memory
    //! \todo Check whether interpolation could be done in the spare time between frames in Graphics method
	class BaseInterpolated : public virtual BaseRenderable, private CallableObject{
	public:

        //! \note This should not be relied on to create BaseRenderable
		DLLEXPORT BaseInterpolated();
        DLLEXPORT virtual ~BaseInterpolated();



        //! \brief Used to intercept frame rendering events and interpolate the position
        DLLEXPORT int OnEvent(Event** pEvent) override;

        //! Not used
		DLLEXPORT int OnGenericEvent(GenericEvent** pevent) override;


        //! \brief Sets initial interpolation variables and starts interpolating
        //! \param originalposition The starting position from which the entity moves smoothly to the actual one
        //! \param originalrotation The starting rotation from which the entity moves smoothly to the actual one
        //! returns True when interpolation is started. False when the positions are so close to the actual ones
        //! that interpolation isn't required
        DLLEXPORT bool StartInterpolating(const Float3 &originalposition, const Float4 &originalrotation);

        //! \brief Stops interpolating
        DLLEXPORT void StopInterpolating(boost::unique_lock<boost::mutex> &lock);

        DLLEXPORT FORCE_INLINE void StopInterpolating(){
            boost::unique_lock<boost::mutex> lock(InterpolationMutex);
            StopInterpolating(lock);
        }

	protected:

        // Inheriting classes need to implement these to make this work with all kinds of object that have some kind
        // of positional data

        //! \brief Called to get the current position for interpolation
        //! \note This entity won't be locked during this call
        virtual void _GetCurrentActualPosition(Float3 &pos) = 0;
        
        //! \brief Called to get the current rotation for interpolation
        //! \note This entity won't be locked during this call
        virtual void _GetCurrentActualRotation(Float4 &rot) = 0;

        void _DoActualInterpolation();
        
        // ------------------------------------ //
        // Variables controlling interpolation //

        //! True when doing interpolation
        bool Interpolating;

        //! The position at which the object appears at
        Float3 InterpolatedPosition;

        //! The position it was at at the last frame.
        //! This is used to make the entity move at a constant speed while interpolating
        Float3 OldActualPosition;

        //! The rotation at which the object appears in
        Float4 InterpolatedRotation;

        //! The rotation it was at at the last frame.
        //! This is used to make the entity move rotate at a constant speed while interpolating
        Float4 OldActualRotation;


        //! Locked when using/changing interpolation variables
        boost::mutex InterpolationMutex;
	};

}
#endif
