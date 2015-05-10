#pragma once
// ------------------------------------ //
#include "Include.h"

#include "../Common/ObjectPool.h"

#include "System.h"
#include "EntityCommon.h"

namespace Leviathan{

    //! \brief Base for all kinds of nodes
	class Node : public ThreadSafe{
	public:

        
	};

    template<class NodeType>
    class NodeHolder : public ObjectPool<NodeType, ObjectID>{
    public:

        //! \brief Runs the system that accepts nodes of the held type
        DLLEXPORT void RunSystem(const System<NodeType> &system){

            GUARD_LOCK();

            for(auto iter = this->Index.begin(); iter != this->Index.end(); ++iter){

                system.ProcessNode(*iter->second, iter->first, *this, guard);
            }
        }
    };
}
