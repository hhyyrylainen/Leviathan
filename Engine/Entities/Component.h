// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"

#include "Common/ObjectPool.h"
#include "Common/SFMLPackets.h"
#include "EntityCommon.h"

#include <limits>

// No clue where this is coming from...
#ifdef _WIN32
#undef max
#endif

namespace Leviathan {

//! Must contain all valid Component types
enum class COMPONENT_TYPE : uint16_t {

    Position,

    //! \todo Hidden as serialized data
    RenderNode,

    Sendable,

    Received,

    Physics,

    BoxGeometry,

    Model,

    // ManualObject,

    Camera,

    // Plane,

    Animated,

    //! All values above this are application specific types
    Custom = 10000
};


//! \brief Base class for all components
class Component {
public:
    inline Component(COMPONENT_TYPE type) : Marked(true), Type(type){};

    //! Set to true when this component has changed
    //! Can be used by other systems to react to changing components
    //! \note This is true when the component has just been created
    bool Marked;

    //! Type of this component, used for network serialization
    const COMPONENT_TYPE Type;

    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;
};

template<class ComponentType>
class ComponentHolder : public ObjectPoolTracked<ComponentType, ObjectID> {
public:
};
} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::ComponentHolder;
#endif // LEAK_INTO_GLOBAL
