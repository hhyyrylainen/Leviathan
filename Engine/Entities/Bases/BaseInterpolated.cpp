// ------------------------------------ //
#ifndef LEVIATHAN_BASEINTERPOLATED
#include "BaseInterpolated.h"
#endif
#include "boost/thread/lock_types.hpp"
#include "OgreSceneNode.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseInterpolated::BaseInterpolated() : BaseRenderable(false){

}

DLLEXPORT Leviathan::BaseInterpolated::~BaseInterpolated(){

    StopInterpolating();
}
// ------------------------------------ //
DLLEXPORT int Leviathan::BaseInterpolated::OnEvent(Event** pEvent){

    assert(Interpolating && "BaseInterpolated should be interpolating in OnEvent");

    _DoActualInterpolation();
    
    // Always valid //
    return 1;
}

DLLEXPORT int Leviathan::BaseInterpolated::OnGenericEvent(GenericEvent** pevent){

    return -1;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseInterpolated::StartInterpolating(const Float3 &originalposition,
    const Float4 &originalrotation)
{
    boost::unique_lock<boost::mutex> lock(InterpolationMutex);

    if(!Interpolating){

        // Start listening for the event //
        RegisterForEvent(EVENT_TYPE_FRAME_BEGIN);
    }
    
    Interpolating = true;

    InterpolatedPosition = originalposition;
    InterpolatedRotation = originalrotation;
}

DLLEXPORT void Leviathan::BaseInterpolated::StopInterpolating(boost::unique_lock<boost::mutex> &lock){


    if(Interpolating){

        UnRegister(EVENT_TYPE_FRAME_BEGIN);
        Interpolating = false;
    }
}
// ------------------------------------ //
void Leviathan::BaseInterpolated::_DoActualInterpolation(){

    boost::unique_lock<boost::mutex> lock(InterpolationMutex);

    Float3 targetpos;
    _GetCurrentActualPosition(targetpos);

    Float4 targetrot;
    _GetCurrentActualRotation(targetrot);

    // Calculate the differences //
    Float3 posdifferences = targetpos-InterpolatedPosition;

    posdifferences /= INTERPOLATION_TARGET_REACH_IN_FRAMES;

    bool poscloseenough = false;
    
    // Quit if close enough //
    if(posdifferences.HAddAbs() <= INTERPOLATION_STOP_PRECISION){

        poscloseenough = true;
        InterpolatedPosition = targetpos;
        
    } else {

        InterpolatedPosition += posdifferences;
    }

    bool rotcloseenough = false;

    if((targetrot-InterpolatedRotation).HAddAbs() <= INTERPOLATION_STOP_PRECISION){

        rotcloseenough = true;
        InterpolatedRotation = targetrot;
        
    } else {

        InterpolatedRotation = InterpolatedRotation.Slerp(targetrot, 1.f/INTERPOLATION_TARGET_REACH_IN_FRAMES);
    }

    // Apply them //
    if(ObjectsNode){

        ObjectsNode->setPosition(InterpolatedPosition);
        ObjectsNode->setOrientation(InterpolatedRotation);
    }

    if(poscloseenough && rotcloseenough){

        StopInterpolating(lock);
    }
}
// ------------------------------------ //



