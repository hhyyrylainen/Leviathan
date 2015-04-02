// ------------------------------------ //
#include "BaseTimedInterpolated.cpp"
using namespace Leviathan;
// ------------------------------------ //
BaseTimedInterpolated::BaseTimedInterpolated() : TimeAccumulator(0){

    CurrentAnimation.Duration = 0;
}
// ------------------------------------ //
void BaseTimedInterpolated::UpdateInterpolation(int mspassed){

    if(CurrentAnimation.Duration < 1)
        return;
    
    TimeAccumulator += mspassed;
    
    const float progress = TimeAccumulator > 0 ? CurrentAnimation.Duration / (float)TimeAccumulator : 0.f;

    SetStateToInterpolated(CurrentAnimation.First, CurrentAnimation.Second, progress);

    if(progress >= 1.f){

        const int wentover = TimeAccumulator-CurrentAnimation.Duration;

        if(OnInterpolationFinished()){

            // We can start interpolating again //
            // This prevents long frames from desyncing interpolation
            UpdateInterpolation(wentover > 0 ? wentover : 0);
        }
    }
}
// ------------------------------------ //
bool BaseTimedInterpolated::OnInterpolationFinished(){

    return false;
}
// ------------------------------------ //
DLLEXPORT bool BaseTimedInterpolated::SetCurrentInterpolation(const ObjectInterpolation &interpolation){

    if(interpolation.Duration < 1 || !interpolation.First || !interpolation.Second)
        return false;

    CurrentAnimation = interpolation;
    
    TimeAccumulator = 0;
    return true;
}
// ------------------------------------ //
