// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Component.h"

namespace Leviathan {

//! \brief Alternative base class for Component that creates distinct state objects
template<class StateT>
class ComponentWithStates : public Component {
public:
    inline ComponentWithStates(COMPONENT_TYPE type) : Component(type) {}

    //! True when there are states in a StateHolder for this entity, which haven't been
    //! shown yet.
    //! \note This is true by default to always apply the initial position even
    //! if there are no states
    bool StateMarked = true;

    // Data for currently interpolating state
    // Some child classes might not use this if interpolating is not done
    // maybe some of these could be moved to a new class
    float InterpolatingStartTime = 0.f;
    int InterpolatingRemoteStartTick;
    StateT* InterpolatingStartState = nullptr;
    StateT* InterpolatingEndState = nullptr;

    // Current time can be calculated from the game world tick and engine clock
    // once the time - InterpolatingStartTime is >= TICKSPEED
    // InterpolatingStartTick is incremented and if there are no state
    // with that tick number StateMarked will be set false and
    // interpolation stops until a new state is a added when we are
    // marked by the state creation and the interpolation continues
    // from the newest tick (number > InterpolatingStartTick)
};

//! \brief Base for all component state classes
class BaseComponentState {
public:
    inline BaseComponentState(int32_t tick, COMPONENT_TYPE componenttype) :
        TickNumber(tick), ComponentType(componenttype)
    {}
    virtual ~BaseComponentState() {}

    inline BaseComponentState(const BaseComponentState& other) noexcept :
        TickNumber(other.TickNumber), ComponentType(other.ComponentType),
        UpdatedFields(other.UpdatedFields)
    {}

    inline BaseComponentState(BaseComponentState&& other) noexcept :
        TickNumber(other.TickNumber), ComponentType(other.ComponentType),
        UpdatedFields(other.UpdatedFields)
    {}

    inline BaseComponentState& operator=(const BaseComponentState& other) noexcept
    {
        TickNumber = other.TickNumber;
        ComponentType = other.ComponentType;
        UpdatedFields = other.UpdatedFields;
        return *this;
    }

    inline BaseComponentState& operator=(BaseComponentState&& other) noexcept
    {
        TickNumber = other.TickNumber;
        ComponentType = other.ComponentType;
        UpdatedFields = other.UpdatedFields;
        return *this;
    }

    //! \brief Adds update data to a packet
    //! \param olderstate The state against which this is compared. Or null if a full update is
    //! wanted
    //! \warning olderstate MUST BE of the same child class as this. Otherwise everything will
    //! explode
    DLLEXPORT virtual void AddDataToPacket(
        sf::Packet& packet, BaseComponentState* olderstate) const = 0;

    //! \brief Copies data to missing values in this state from another state
    //! \return True if all missing values have been filled
    //! \warning otherstate MUST BE of the same child class as this. Otherwise everything will
    //! explode
    DLLEXPORT virtual bool FillMissingData(BaseComponentState& otherstate) = 0;

    //! \brief Returns true if bitnum bit is set in UpdatedFields
    //! \param BitNum the index of the bit to set. Starts at 0 and last valid is 7
    inline bool IsBitSet(uint8_t bitnum = 0) const
    {
        return (UpdatedFields & (1 << bitnum)) == 1;
    }

    //! \brief Sets bitnum bit in Updated
    //! \see IsBitSet
    inline void SetBit(uint8_t bitnum = 0)
    {
        UpdatedFields |= (1 << bitnum);
    }

    inline void SetAllBitsInUpdated()
    {
        UpdatedFields = std::numeric_limits<uint8_t>::max();
    }

public:
    //! The tick this state was captured on
    //! This is used for rebuilding delta states
    int32_t TickNumber;

    //! The component type. This is needed to handle the actual subclasses that have data
    //! without having to do virtual method calls to find matching types
    COMPONENT_TYPE ComponentType;

    //! Some components have properties that don't change very often so here is reserved 8 bits
    //! to let the state object have more granularity as to which parts of it need to be sent
    //! over the network
    uint8_t UpdatedFields = std::numeric_limits<uint8_t>::max();
};

//! \brief Class for supporting script defined component states
//! \todo This isn't implemented
class ScriptComponentState : public BaseComponentState {};

//! \brief Holder of state for a whole entity
class EntityState {
public:
    //! \brief Creates a delta update to packet
    DLLEXPORT void CreateUpdatePacket(EntityState& olderstate, sf::Packet& packet);

    //! \brief Adds full data to packet
    //! \note This is a separate method from CreateUpdatePacket because this is more efficient
    //! as this doesn't try to match up the different component states
    DLLEXPORT void AddDataToPacket(sf::Packet& packet);


    inline void Append(std::unique_ptr<BaseComponentState>&& state)
    {
        ComponentStates.push_back(std::move(state));
    }

    std::vector<std::unique_ptr<BaseComponentState>> ComponentStates;
};



} // namespace Leviathan
