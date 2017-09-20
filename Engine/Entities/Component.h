// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"

#include "Common/ObjectPool.h"
#include "Common/SFMLPackets.h"
#include "EntityCommon.h"

#include <limits>

namespace Leviathan{

//! Must contain all valid Component types
enum class COMPONENT_TYPE : uint16_t{
    
    Position,

    //! \todo Hidden as serialized data
    RenderNode,

    Sendable,

    Received,

    Physics,

    BoxGeometry,

    Model,

    ManualObject,

    Camera,

    Plane,

    //! All values above this are application specific types
    Custom = 10000
};


//! \brief Base class for all components
class Component{
public:

    inline Component(COMPONENT_TYPE type) : Marked(true), Type(type){};

    //! Set to true when this component has changed
    //! Can be used by other systems to react to changing components
    //! \note This is true when the component has just been created
    bool Marked;

    //! Type of this component, used for network serialization
    const COMPONENT_TYPE Type;
    
    Component(const Component&) = delete;
    Component& operator =(const Component&) = delete;
};

//! \brief Base class for all component data
//!
//! Used to force all components to define a serializer for its data
struct ComponentData {
    
    
};

//! \brief Base class for storing component states
class ComponentState {
public:

    //! \brief Used to hold data with an updated flag to support partial states
    //! \todo Replace with a bitfield in ComponentState that automatically is configured
    //! to be large enough
    template<typename T>
        struct PotentiallyUpdatedValue {
        
        inline PotentiallyUpdatedValue(){}
        inline PotentiallyUpdatedValue(const T &value) : Value(value), Updated(1){}

        //! \returns True if Updated
        inline operator bool(){
            return Updated;
        }

        //! \brief Sets a new value and sets as Updated
        inline PotentiallyUpdatedValue& operator=(const T &value){

            Updated = true;
            Value = value;
            return *this;
        }

        //! \brief Returns true if BitNum bit is set in Updated
        inline bool IsBitSet(uint8_t BitNum = 0) const
        {
            return (Updated & (1 << BitNum)) == 1;
        }

        //! \brief Sets BitNum bit in Updated
        inline void SetBit(uint8_t BitNum = 0)
        {
            Updated |= (1 << BitNum);
        }

        //! \brief Returns true if up and including BitNum is set in Updated
        //! \todo Verify that tail call optimization kicks in and this is inlined
        inline bool BitsSetUntil(uint8_t BitNum) const
        {
            if(BitNum == 0){
                
                return IsBitSet(0);
                
            } else {

                return IsBitSet(0) && BitsSetUntil(BitNum - 1);
            }
        }

        inline void SetAllBitsInUpdated(){

            Updated = std::numeric_limits<uint8_t>::max();
        }

        //! Marks whether it is updated or not
        //! For basic usage 0 means not updated 1 means it is updated
        //! \detail More specific usage can use the individual bytes to check
        //! whether subcomponents are updated (Like Float3 individual values)
        uint8_t Updated = 0;
        T Value;
    };



public:

    template<typename T>
        using Member = PotentiallyUpdatedValue<T>;
    
    inline ComponentState(int32_t tick, COMPONENT_TYPE componenttype) :
        Tick(tick), ComponentType(componenttype){}
    virtual ~ComponentState(){}

    //! \brief Adds update data to a packet
    //! \param olderstate The state against which this is compared.
    //! Or NULL if a full update is wanted
    DLLEXPORT virtual void CreateUpdatePacket(ComponentState* olderstate,
        sf::Packet &packet) = 0;

    //! \brief Copies data to missing values in this state from another state
    //! \return True if all missing values have been filled
    DLLEXPORT virtual bool FillMissingData(ComponentState &otherstate) = 0;

    //! \brief The tick this delta state matches
    const int32_t Tick;

    const COMPONENT_TYPE ComponentType;
        
    ComponentState(const ComponentState &other) = delete;
    void operator=(const ComponentState &other) = delete;
};


template<class ComponentType>
    class ComponentHolder : public ObjectPool<ComponentType, ObjectID>{
public:
    
    
    

};
}
