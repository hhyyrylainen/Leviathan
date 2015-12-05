#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "../Common/ThreadSafe.h"
#include "Exceptions.h"

namespace Leviathan{

    using ObjectID = int;

    template<class NodeType> class NodeHolder;
    
    //! \brief Base for all entity component related systems
    template<class UsedNode>
	class System{
	public:

        //! \brief Called for each node of the matching type when this system is ran
        //! \param node The node that needs to be processed
        //! \param ObjectID The ID of the entity that owns the components in node
        //! \param pool The object containing all nodes of this type, can be used to find other
        //! nodes of the same type
        //! \param poollock The lock for the object pool that owns the node, pass when finding
        //! more elements of the same type
        DLLEXPORT virtual void ProcessNode(UsedNode &node, ObjectID nodesobject,
            NodeHolder<UsedNode> &pool, Lock &poollock) const
        {
            throw Exception("Base System ProcessNode called");
        }
	};
}


