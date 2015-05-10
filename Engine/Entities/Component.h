#pragma once
// ------------------------------------ //
#include "Include.h"

#include "../Common/ObjectPool.h"
#include "EntityCommon.h"

namespace Leviathan{

    class Component : public ThreadSafe{
    public:

        //! Set to true when this component has changed
        //! Can be used by other systems to react to changing components
        bool Marked;
    };

    template<class ComponentType>
    class ComponentHolder : public ObjectPool<ComponentType, ObjectID>{
    public:

        


    };
}
