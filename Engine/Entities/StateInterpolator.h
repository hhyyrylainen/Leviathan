// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "StateHolder.h"

namespace Leviathan{

class StateInterpolator{
public:

    //! \brief Interpolates states for component.
    //!
    //! Will update the interpolation variables in the component
    //! (which should be derived from ComponentWithStates) Our caller
    //! should also check ComponentWithStates::StateMarked before
    //! calling us
    //! \returns Tuple of state valid and state. If the bool is false StateT will be garbage
    template<class StateT, class ComponentT>
        static std::tuple<bool, StateT> Interpolate(const StateHolder<StateT> &stateholder,
            ObjectID entity, ComponentT* entitycomponent, int currenttick, int timeintick)
    {
        // TODO: should this be stored in the component?
        auto* entitysStates = stateholder.GetEntityStates(entity);

        if(!entitysStates){
            // Probably shouldn't throw here to make the code that uses this simpler
            entitycomponent->StateMarked = false;
            return std::make_tuple(false, StateT());
            //throw Leviathan::InvalidState("Interpolated entity has no states in StateHolder");
        }

        // Find interpolation start spot //
        if(!entitycomponent->InterpolatingStartState ||
            !entitysStates->IsStateValid(entitycomponent->InterpolatingStartState))
        {
            entitycomponent->InterpolatingEndState = nullptr;
            entitycomponent->InterpolatingStartState = entitysStates->GetOldest();

            if(!entitycomponent->InterpolatingStartState){

                // No states to interpolate //
                entitycomponent->StateMarked = false;
                return std::make_tuple(false, StateT());
            }

            // Adjust clock if the initial tick has been changed //
            if(entitycomponent->InterpolatingStartTime != 0.f){

                if(entitycomponent->InterpolatingRemoteStartTick !=
                    entitycomponent->InterpolatingStartState->TickNumber)
                {
                    AdjustClock(entitycomponent);
                }
            }
        }

        // We cast the time from int64_t to float (visual studio was giving warnings about 
        // losing data here)
        const float currentTime = static_cast<float>((currenttick * TICKSPEED) + timeintick);

        // Find ending state //
        if(!entitycomponent->InterpolatingEndState){

            // TODO: should we only allow TickNumber + 2 states to be interpolated to
            // as that is the way source engine does it and would avoid jitter if we miss
            // one state packet later (use INTERPOLATION_TIME / TICKSPEED ?)
            entitycomponent->InterpolatingEndState = entitysStates->GetMatchingOrNewer(
                entitycomponent->InterpolatingStartState->TickNumber + 1);

            if(!entitycomponent->InterpolatingEndState){

                // No ending state found //
                entitycomponent->StateMarked = false;
                // Only one state should allow interpolating to the one available state
                // with the same function so we return the first state here
                //return std::make_tuple(false, StateT());
                return std::make_tuple(true, *entitycomponent->InterpolatingStartState);
            }

            // Initialize the remote time counter if this is the first time we start
            // interpolating
            if(entitycomponent->InterpolatingStartTime == 0.f){
                entitycomponent->InterpolatingStartTime = currentTime;
                entitycomponent->InterpolatingRemoteStartTick =
                    entitycomponent->InterpolatingStartState->TickNumber;
            }
        }

        const float passed = currentTime - entitycomponent->InterpolatingStartTime;

        // TODO: do we need to check for currentTime < 0?
        
        if(passed <= EPSILON)
            return std::make_tuple(true, *entitycomponent->InterpolatingStartState);

        // Duration is clamped to INTERPOLATION_TIME to make entities
        // that have stopped moving not take a ridiculously long time
        // to move to their new positions
        const auto duration = std::min((
            entitycomponent->InterpolatingEndState->TickNumber -
            entitycomponent->InterpolatingStartState->TickNumber) * TICKSPEED,
            INTERPOLATION_TIME);
        
        if(passed == duration)
            return std::make_tuple(true, *entitycomponent->InterpolatingEndState);

        // Check for having finished interpolating //
        if(passed > duration){

            entitycomponent->InterpolatingStartState = entitycomponent->InterpolatingEndState;
            entitycomponent->InterpolatingEndState = nullptr;

            AdjustClock(entitycomponent);

            // We need to recurse to get the correct interpolation state //
            return Interpolate(stateholder, entity, entitycomponent, currenttick, timeintick);
        }
        
        const float progress = passed / duration;
        
        return std::make_tuple(true, entitycomponent->InterpolatingStartState->Interpolate(
                *entitycomponent->InterpolatingEndState, progress));
    }

    template<class ComponentT>
        static void AdjustClock(ComponentT &entitycomponent)
    {
        const auto difference = entitycomponent->InterpolatingStartState->TickNumber
            - entitycomponent->InterpolatingRemoteStartTick;

        entitycomponent->InterpolatingStartTime += difference * TICKSPEED;
        entitycomponent->InterpolatingRemoteStartTick =
            entitycomponent->InterpolatingStartState->TickNumber;
    }
    
};




}

