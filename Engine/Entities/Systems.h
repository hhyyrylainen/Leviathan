// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once

//! \file Contains all common systems that GameWorld will run on its components
//! at specified times

// ------------------------------------ //
#include "Include.h"

#include "Components.h"
#include "StateInterpolator.h"
#include "System.h"

#include "Utility/Convert.h"

#include "Generated/ComponentStates.h"

#include "OgreSceneNode.h"

namespace Leviathan {

// ------------------------------------ //
// State creation systems
class PositionStateSystem : public StateCreationSystem<Position, PositionState> {
};


// ------------------------------------ //

//! \brief Moves nodes of entities that have their positions changed
class RenderingPositionSystem : public System<std::tuple<RenderNode&, Position&>> {

    void ProcessNode(std::tuple<RenderNode&, Position&>& node, ObjectID id,
        const StateHolder<PositionState>& heldstates, int tick, int timeintick)
    {
        auto& pos = std::get<1>(node);

        if(!pos.StateMarked)
            return;

        auto interpolated =
            StateInterpolator::Interpolate(heldstates, id, &pos, tick, timeintick);

        auto& rendernode = std::get<0>(node);

        if(!std::get<0>(interpolated)) {
            // No states to interpolate //
            rendernode.Node->setPosition(pos.Members._Position);
            rendernode.Node->setOrientation(pos.Members._Orientation);
            return;
        }

        const auto& state = std::get<1>(interpolated);
        rendernode.Node->setPosition(state._Position);
        rendernode.Node->setOrientation(state._Orientation);
    }

public:
    template<class GameWorldT>
    void Run(GameWorldT& world, const StateHolder<PositionState>& heldstates, int tick,
        int timeintick)
    {
        auto& index = CachedComponents.GetIndex();
        for(auto iter = index.begin(); iter != index.end(); ++iter) {

            this->ProcessNode(*iter->second, iter->first, heldstates, tick, timeintick);
        }
    }

    //! \brief Creates nodes if matching ids are found in all data vectors or
    //! already existing component holders
    //!
    //! \todo This should probably not be templated
    void CreateNodes(const std::vector<std::tuple<RenderNode*, ObjectID>>& firstdata,
        const std::vector<std::tuple<Position*, ObjectID>>& seconddata,
        const ComponentHolder<RenderNode>& firstholder,
        const ComponentHolder<Position>& secondholder)
    {
        TupleCachedComponentCollectionHelper(
            CachedComponents, firstdata, seconddata, firstholder, secondholder);
    }

    //! \brief Destroys nodes if matching ids are found in the removed data data vectors
    void DestroyNodes(const std::vector<std::tuple<RenderNode*, ObjectID>>& firstdata,
        const std::vector<std::tuple<Position*, ObjectID>>& seconddata)
    {
        CachedComponents.RemoveBasedOnKeyTupleList(firstdata);
        CachedComponents.RemoveBasedOnKeyTupleList(seconddata);
    }
};

//! \brief Handles properties of Ogre nodes that have a changed RenderNode
class RenderNodePropertiesSystem {
public:
    void Run(GameWorld& world, std::unordered_map<ObjectID, RenderNode*>& index)
    {
        for(auto iter = index.begin(); iter != index.end(); ++iter) {

            auto& node = *iter->second;

			if (node.Marked) {
				// TODO: would it be faster to first check have these
				// changed or is it better to just set them as Ogre might
				// also check have the value changed
				node.Node->setVisible(!node.Hidden);
				node.Node->setScale(node.Scale);

				node.Marked = false;
			}
        }
    }
};


//! \brief Handles updating time of Ogre animations
class AnimationTimeAdder {
public:
    DLLEXPORT void Run(GameWorld& world, std::unordered_map<ObjectID, Animated*>& index,
        int tick, int timeintick);

private:
    float LastSeconds = 0;
};


//! \brief Sends updated entities from server to clients
//! \todo Change this to take distance into account
//! don't send as many updates to clients far away
class SendableSystem {
public:
    //! \pre Final states for entities have been created for current tick
    void Run(std::unordered_map<ObjectID, Sendable*>& index, GameWorld& world)
    {

        for(auto iter = index.begin(); iter != index.end(); ++iter) {

            auto& node = *iter->second;

            if(!node.Marked)
                return;

            HandleNode(iter->first, node, world);

            node.Marked = false;
        }
    }

protected:
    DLLEXPORT void HandleNode(ObjectID id, Sendable& obj, GameWorld& world);
};

//! \brief Interpolates states for received objects and handles locally controlled entities
class ReceivedSystem {
public:
    DLLEXPORT void Run(std::unordered_map<ObjectID, Received*>& index, GameWorld& world);
};
} // namespace Leviathan
