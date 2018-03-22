// ------------------------------------ //
#include "Systems.h"

#include "CommonStateObjects.h"
#include "GameWorld.h"
#include "Networking/Connection.h"
#include "Networking/NetworkHandler.h"
#include "Networking/NetworkRequest.h"
#include "Networking/NetworkResponse.h"
#include "Networking/SentNetworkThing.h"

#include "Animation/OgreSkeletonAnimation.h"
#include "Animation/OgreSkeletonInstance.h"
#include "OgreItem.h"
using namespace Leviathan;

// ------------------ SendableSystem ------------------ //
DLLEXPORT void HandleNode(ObjectID id, Sendable& obj, GameWorld& world)
{

    const auto ticknumber = world.GetTickNumber();

    // Create current state here as one or more connections should require it //
    std::shared_ptr<ComponentState> curstate;

    DEBUG_BREAK;

    if(!curstate) {

        Logger::Get()->Error(
            "SendableSystem: created invalid state for entity, id: " + Convert::ToString(id));

        obj.Marked = false;
        return;
    }

    for(auto iter = obj.UpdateReceivers.begin(); iter != obj.UpdateReceivers.end();) {

        auto connection = (*iter)->CorrespondingConnection;

        // Find the receiver from the world //
        // And fail if not found or the connection is not open
        if(!world.IsConnectionInWorld(*connection) || !connection->IsValidForSend()) {

            // Connection no longer updated for world //
            iter = obj.UpdateReceivers.erase(iter);
            continue;
        }

        // Check has some of the packets been received //
        (*iter)->CheckReceivedPackets();

        // Prepare the packet //
        sf::Packet updatedata;

        // Count the components that have states //
        DEBUG_BREAK;

        // Do not use last confirmed if it is too old
        int referencetick = -1;

        if(ticknumber <
            (*iter)->LastConfirmedTickNumber + BASESENDABLE_STORED_RECEIVED_STATES - 1) {
            referencetick = (*iter)->LastConfirmedTickNumber;

            // Now calculate a delta update from curstate to the last confirmed state //
            DEBUG_BREAK;
            // curstate->CreateUpdatePacket((*iter)->LastConfirmedData.get(), *packet.get());

        } else {

            // Data is too old and cannot be used //
            DEBUG_BREAK;
            // curstate->CreateUpdatePacket(nullptr, *packet.get());

            (*iter)->LastConfirmedTickNumber = -1;
            (*iter)->LastConfirmedData.reset();
        }

        // Create the final update packet //
        auto sentthing = connection->SendPacketToConnection(
            std::make_shared<ResponseEntityUpdate>(
                0, world.GetID(), ticknumber, referencetick, id, std::move(updatedata)),
            RECEIVE_GUARANTEE::None);

        // Add a callback for success //
        (*iter)->AddSentPacket(ticknumber, curstate, sentthing);

        ++iter;
    }
}
// ------------------------------------ //
DLLEXPORT void ReceivedSystem::Run(
    std::unordered_map<ObjectID, Received*>& Index, GameWorld& world)
{
    const float progress = world.GetTickProgress();
    const auto tick = world.GetTickNumber();

    for(auto iter = Index.begin(); iter != Index.end(); ++iter) {

        auto& node = *iter->second;

        // Unmarked nodes should have invalid interpolation status
        if(!node.Marked)
            return;

        if(!node.LocallyControlled) {

            // Interpolate received states //
            float adjustedprogress = progress;
            const Received::StoredState* first;
            const Received::StoredState* second;

            try {
                node.GetServerSentStates(&first, &second, tick, adjustedprogress);
            } catch(const InvalidState&) {

                // If not found unmark to avoid running unneeded //
                node.Marked = false;
                continue;
            }

            DEBUG_BREAK;
            // first->Interpolate(second, adjustedprogress);

        } else {

            // Send updates to the server //
        }
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SendableSystem::HandleNode(
    ObjectID id, Sendable& obj, GameWorld& world){DEBUG_BREAK}

// ------------------------------------ //
// AnimationTimeAdder
DLLEXPORT void AnimationTimeAdder::Run(
    GameWorld& world, std::unordered_map<ObjectID, Animated*>& index, int tick, int timeintick)
{
    float timeNow = (tick * TICKSPEED + timeintick) / 1000.f;

    const float passed = timeNow - LastSeconds;

    LastSeconds = timeNow;

    for(auto iter = index.begin(); iter != index.end(); ++iter) {

        Animated& animated = *iter->second;

        if(!animated.GraphicalObject)
            continue;

        auto skeleton = animated.GraphicalObject->getSkeletonInstance();

        if(!skeleton) {

            LOG_WARNING("Animated (entity id: " + std::to_string(iter->first) +
                        ") has an Item that has no skeleton instance");
            continue;
        }

        if(animated.Marked) {
            animated.Marked = false;

            // Apply all properties and stop not playing animations
            for(const auto& animation : animated.Animations) {

                try {

                    // Documentation says that this throws if not found
                    Ogre::SkeletonAnimation* ogreAnim = skeleton->getAnimation(animation.Name);

                    ogreAnim->setEnabled(true);
                    ogreAnim->setLoop(animation.Loop);

                } catch(const Ogre::Exception& e) {

                    LOG_WARNING("Animated (entity id: " + std::to_string(iter->first) +
                                ") has no animation named: " + animation.ReadableName +
                                ", exception: " + e.what());
                }
            }

            // Then disable
            for(const auto& ogreAnimation : skeleton->getAnimations()) {

                bool found = false;

                for(const auto& animation : animated.Animations) {

                    if(animation.Name == ogreAnimation.getName()) {
                        found = true;
                        break;
                    }
                }

                if(!found) {

                    skeleton->getAnimation(ogreAnimation.getName())->setEnabled(false);
                }
            }
        }

        // Update animation time
        for(const auto& animation : animated.Animations) {

            if(animation.Paused)
                return;

            try {

                // Documentation says that this throws if not found
                Ogre::SkeletonAnimation* ogreAnim = skeleton->getAnimation(animation.Name);
                ogreAnim->addTime(passed * animation.SpeedFactor);

            } catch(const Ogre::Exception& e) {

                LOG_WARNING("Animated (entity id: " + std::to_string(iter->first) +
                            ") has no animation named (in update): " + animation.ReadableName +
                            ", exception: " + e.what());
            }
        }
    }
}
