// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ObjectPool.h"
#include "Common/SFMLPackets.h"
#include "EntityCommon.h"

#include <array>
#include <memory>

namespace Leviathan {

//! Number of states that are kept.
//! Needs to be high enough that no currently interpolating states are dumped (unless we are in
//! some special condition where our client side clock is messed up or we are lagging)
constexpr auto KEPT_STATES_COUNT = 5;

template<class StateT>
class StateHolder;

//! \brief Holds state objects of type StateT related to a single entity
template<class StateT>
class ObjectsComponentStates {
public:
    //! \brief Returns the state with the time
    StateT* GetNewest() const
    {
        StateT* newest = nullptr;
        float highest = 0;

        for(StateT* state : StoredStates) {

            if(state && state->StateTime >= highest) {

                newest = state;
                highest = state->StateTime;
            }
        }

        return newest;
    }

    //! \brief Returns the state with the lowest time
    StateT* GetOldest() const
    {
        StateT* oldest = nullptr;
        float lowest = std::numeric_limits<float>::max();

        for(StateT* state : StoredStates) {

            if(state && state->StateTime <= lowest) {

                oldest = state;
                lowest = state->StateTime;
            }
        }

        return oldest;
    }

    //! \brief Returns the state with closest time that is higher
    //! \todo How can the caller of this method detect when a state
    //! has been missed and should instead (maybe) wait a bit? Or
    //! should we just skip missed states and start interpolating from
    //! the later state
    StateT* GetNewer(float time) const
    {
        StateT* closest = nullptr;
        float closestBy = std::numeric_limits<float>::max();

        for(StateT* state : StoredStates) {

            if(!state || state->StateTime <= time)
                continue;

            const auto difference = state->StateTime - time;
            if(difference <= closestBy) {

                closestBy = difference;
                closest = state;
            }
        }

        return closest;
    }

    //! \brief Returns state matching the time
    StateT* GetState(float time) const
    {
        for(StateT* state : StoredStates) {

            if(state && state->StateTime == time)
                return state;
        }

        return nullptr;
    }

    //! \brief Returns true if state is still valid
    bool IsStateValid(StateT* statetocheck) const
    {
        for(StateT* state : StoredStates) {
            if(state == statetocheck)
                return true;
        }

        return false;
    }

    //! \brief Appends a new state removing old states if they don't fit anymore
    //! \todo It is probably more efficient to use a swap algorithm for ading new states
    //! instead of this
    void Append(StateT* newstate, StateHolder<StateT>& stateowner)
    {
        // Fill from the back and pop from the front if states don't fit //

        // This would only speed up the first added state, so maybe the performance
        // is better if we skip this check
        // if(StoredStates[KEPT_STATES_COUNT - 1] == nullptr){

        //     StoredStates[KEPT_STATES_COUNT - 1] = newstate;
        //     return;
        // }

        // First state will be popped off if it exists //
        if(StoredStates[0] != nullptr) {

            stateowner._DestroyStateObject(StoredStates[0]);
        }

        for(size_t i = 0; i < KEPT_STATES_COUNT - 1; ++i) {

            StoredStates[i] = StoredStates[i + 1];
        }

        StoredStates[KEPT_STATES_COUNT - 1] = newstate;
    }

    //! \brief Counts the filled number of state slots
    //! \returns A number in range [0, KEPT_STATES_COUNT]
    auto GetNumberOfStates() const
    {
        int count = 0;

        for(StateT* state : StoredStates) {

            if(state)
                ++count;
        }

        return count;
    }

protected:
    std::array<StateT*, KEPT_STATES_COUNT> StoredStates;
};

//! \brief Holds state objects of type for quick access by ObjectID
//! \todo The GameWorld needs to notify this when ObjectID is deleted
template<class StateT>
class StateHolder {
    friend ObjectsComponentStates<StateT>;

public:
    StateHolder() {}

    ~StateHolder()
    {
        // Both of the pools are released here so the destructor of ObjectsComponentStates
        // doesn't need to notify us of the states it held
    }

    //! \brief Creates a new state for entity's component if it has changed
    //! \returns True if a new state was created
    template<class ComponentT>
    bool CreateStateIfChanged(ObjectID id, const ComponentT& component, float time)
    {
        auto entityStates = GetStateFor(id);

        // Get latest state to compare current values against //
        StateT* latestState = entityStates->GetNewest();

        // If empty always create //
        if(!latestState) {

            StateT* newState = _CreateNewState(time, component);
            entityStates->Append(newState, *this);
            return true;
        }

        // Check is the latest state still correct //
        if(latestState->DoesMatchState(component))
            return false;

        // Create a new state //
        StateT* newState = _CreateNewState(time, component);
        entityStates->Append(newState, *this);
        return true;
    }

    //! \brief Deserializes a state for entity's component from an archive
    void DeserializeState(ObjectID id, float time, sf::Packet& data, float referencestatestime)
    {
        auto entityStates = GetStateFor(id);

        this->_DeserializeState(entityStates, id, time, data, referencestatestime);
    }

    //! \brief Deserializes a state for entity's component from an archive and applies it if it
    //! is the newest
    template<class ComponentT>
    void DeserializeAndApplyState(ObjectID id, ComponentT& component, float time,
        sf::Packet& data, float referencestatestime)
    {
        auto entityStates = GetStateFor(id);

        // Deserialize
        auto* deserialized =
            this->_DeserializeState(entityStates, id, time, data, referencestatestime);

        if(!deserialized) {
            LOG_ERROR("StateHolder: failed to deserialize state");
            return;
        }

        // And apply it if it is newest
        auto* newest = entityStates->GetNewest();

        if(deserialized == newest) {

            component.ApplyState(*newest);
        } else {
            float newestNumber = newest ? newest->StateTime : -1;

            LOG_WARNING("StateHolder: DeserializeAndApplyState: received not the newest "
                        "packet, received: " +
                        std::to_string(deserialized->StateTime) +
                        ", newest: " + std::to_string(newestNumber));
        }
    }

    //! \brief Creates a state object for sending
    template<class ComponentT>
    StateT CreateStateForSending(const ComponentT& component, float time) const
    {
        return StateT(time, component);
    }


    //! \brief Returns the number of entities that have states
    auto GetNumberOfEntitiesWithStates() const
    {
        return StateObjects.GetObjectCount();
    }

    //! \brief Returns a pointer to entity's states if they exist
    ObjectsComponentStates<StateT> const* GetEntityStates(ObjectID id) const
    {
        return StateObjects.Find(id);
    }

protected:
    inline StateT* _DeserializeState(ObjectsComponentStates<StateT>* entityStates, ObjectID id,
        float time, sf::Packet& data, float referencestatestime)
    {
        StateT* reference = nullptr;

        if(referencestatestime >= 0.f) {
            reference = entityStates->GetState(referencestatestime);

            if(!reference) {

                LOG_WARNING("StateHolder: DeserializeState: can't find reference tick: " +
                            std::to_string(referencestatestime) +
                            " for entity: " + std::to_string(id));

                reference = entityStates->GetNewest();
            }
        }

        // Create a new state //
        StateT* newState = _CreateNewState(reference, data);

        // This can't be deserialized from data so we forward it
        newState->StateTime = time;

        // TODO: do we need to check whether the states already contain a state for this time
        entityStates->Append(newState, *this);
        return newState;
    }

    inline ObjectsComponentStates<StateT>* GetStateFor(ObjectID id)
    {
        ObjectsComponentStates<StateT>* entityStates = StateObjects.Find(id);

        if(!entityStates) {

            entityStates = StateObjects.ConstructNew(id);
        }

        return entityStates;
    }

    template<typename... Args>
    StateT* _CreateNewState(Args&&... args)
    {
        return StatePool.ConstructNew(std::forward<Args>(args)...);
    }

    void _DestroyStateObject(StateT* state)
    {
        StatePool.Destroy(state);
    }

private:
    //! Keeps track of states associated with an object
    ObjectPool<ObjectsComponentStates<StateT>, ObjectID> StateObjects;

    //! All of the created StateT objects are held by this pool and
    //! only referenced from StateObjects
    BasicPool<StateT> StatePool;
};

} // namespace Leviathan
