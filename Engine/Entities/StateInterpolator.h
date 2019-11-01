// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "StateHolder.h"

namespace Leviathan {

class StateInterpolator {
public:
    //! \brief Interpolates states for component.
    //!
    //! Will update the interpolation variables in the component
    //! (which should be derived from ComponentWithStates) Our caller
    //! should also check ComponentWithStates::StateMarked before
    //! calling us
    //! \returns Tuple of state valid and state. If the bool is false StateT will be garbage
    //! \todo Would it make more sense to have the current world time as a parameter to? As
    //! interpolating to previous time might be needed at some point? Or as a separate method?
    template<class StateT, class ComponentT>
    static std::tuple<bool, StateT> Interpolate(const StateHolder<StateT>& stateholder,
        ObjectID entity, ComponentT* entitycomponent, float elapsed)
    {
        // TODO: should this be stored in the component?
        auto* entitysStates = stateholder.GetEntityStates(entity);

        if(!entitysStates) {
            // Probably shouldn't throw here to make the code that uses this simpler
            entitycomponent->StateMarked = false;
            return std::make_tuple(false, StateT());
            // throw Leviathan::InvalidState("Interpolated entity has no states in
            // StateHolder");
        }

        // Find interpolation start spot //
        if(!entitycomponent->InterpolatingStartState ||
            !entitysStates->IsStateValid(entitycomponent->InterpolatingStartState)) {
            entitycomponent->InterpolatingEndState = nullptr;
            entitycomponent->InterpolatingStartState = entitysStates->GetOldest();

            if(!entitycomponent->InterpolatingStartState) {

                // No states to interpolate //
                entitycomponent->StateMarked = false;
                entitycomponent->TimeSinceStartState = 0;
                return std::make_tuple(false, StateT());
            }
        }

        // Find ending state //
        if(!entitycomponent->InterpolatingEndState) {

            // TODO: apply INTERPOLATION_TIME here to only start interpolation when we have
            // many states

            // TODO: should we only allow TickNumber + 2 states to be interpolated to
            // as that is the way source engine does it and would avoid jitter if we miss
            // one state packet later (use INTERPOLATION_TIME / TICKSPEED ?)
            entitycomponent->InterpolatingEndState =
                entitysStates->GetNewer(entitycomponent->InterpolatingStartState->StateTime);

            if(!entitycomponent->InterpolatingEndState) {

                // No ending state found //
                entitycomponent->StateMarked = false;
                // Not sure if this time reset here is a good idea or not...
                entitycomponent->TimeSinceStartState = 0;
                // Only one state should allow interpolating to the one available state
                // with the same function so we return the first state here
                // return std::make_tuple(false, StateT());
                return std::make_tuple(true, *entitycomponent->InterpolatingStartState);
            }
        }

        entitycomponent->TimeSinceStartState += elapsed;

        // Duration is clamped to INTERPOLATION_TIME to make entities
        // that have stopped moving (and then started again) not take a ridiculously long time
        // to move to their new positions
        const auto duration = std::min(entitycomponent->InterpolatingEndState->StateTime -
                                           entitycomponent->InterpolatingStartState->StateTime,
            INTERPOLATION_TIME);

        // This is a very unlikely case
        if(entitycomponent->TimeSinceStartState == duration)
            return std::make_tuple(true, *entitycomponent->InterpolatingEndState);

        // Check for having finished interpolating //
        if(entitycomponent->TimeSinceStartState > duration) {

            // Hold over time to the next interpolation interval
            entitycomponent->TimeSinceStartState -= duration;

            entitycomponent->InterpolatingStartState = entitycomponent->InterpolatingEndState;
            entitycomponent->InterpolatingEndState = nullptr;

            // We need to recurse to get the correct interpolation state //
            // We pass zero here as no additional time has passed
            return Interpolate(stateholder, entity, entitycomponent, 0.f);
        }

        const float progress = entitycomponent->TimeSinceStartState / duration;

        return std::make_tuple(true, entitycomponent->InterpolatingStartState->Interpolate(
                                         *entitycomponent->InterpolatingEndState, progress));
    }
};




} // namespace Leviathan
