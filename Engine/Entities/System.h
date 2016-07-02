// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"

#include "EntityCommon.h"

#include "Common/ObjectPool.h"
#include "Exceptions.h"


namespace Leviathan{

template<class NodeType>
    class NodeHolder : public ObjectPool<NodeType, ObjectID>{
public:

};
    
//! \brief Base for all entity component related systems
//!
//! For ones that use nodes. Not for ones that directly use a single component type
template<class UsedNode>
	class System{
public:

    using HolderType = NodeHolder<UsedNode>;

    //! \brief Runs this system on its nodes
    //!
    //! \note The nodes need to be updated before calling this, otherwise some entities
    //! might not be picked up
    virtual void Run(GameWorld &world) = 0;

protected:

    //template <typename T, typename R, typename ...Args>
    //    R proxycall(T & obj, R (T::*mf)(Args...), Args &&... args)
    
    //! \brief Helper for Run
    //!
    //! Goes through all nodes and calls func on them
    template <class T, void(T::*F)(UsedNode &node, ObjectID nodesobject)>
        void RunAllNodes(T &instance)
    {
        for(auto iter = Nodes.Index.begin(); iter != Nodes.Index.end(); ++iter){

            instance.F(*iter->second, iter->first);
        }
    }
    
public:
    
    HolderType Nodes;
};

//! \brief Base class for systems that use a single component directly
template<class UsedComponent>
class SingleSystem{
public:
    virtual void Run(std::unordered_map<ObjectID, UsedComponent*> &Index,
        GameWorld &world) = 0;
};

}


