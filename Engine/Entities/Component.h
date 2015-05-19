#pragma once
// ------------------------------------ //
#include "Include.h"

#include "../Common/ObjectPool.h"
#include "EntityCommon.h"

namespace Leviathan{

    class Component : public ThreadSafe{
    public:

        DLLEXPORT Component() : Marked(true){};

        //! Set to true when this component has changed
        //! Can be used by other systems to react to changing components
        //! \note This is true when the component has just been created
        //! \todo Make this an atomic
        bool Marked;

        Component(const Component&) = delete;
        Component& operator =(const Component&) = delete;
    };

    template<class ComponentType>
    class ComponentHolder : public ObjectPool<ComponentType, ObjectID>{
    public:

        


    };
}
