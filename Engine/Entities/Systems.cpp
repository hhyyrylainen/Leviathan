// ------------------------------------ //
#include "Systems.h"

#include "GameWorld.h"
#include "Networking/Connection.h"
#include "Networking/NetworkHandler.h"
#include "Networking/NetworkRequest.h"
#include "Networking/NetworkResponse.h"
#include "Networking/SentNetworkThing.h"

#include "Engine.h"
#include "Rendering/Graphics.h"

#include "bsfCore/Components/BsCAnimation.h"
#include "bsfCore/Components/BsCRenderable.h"

using namespace Leviathan;
// ------------------------------------ //
// ModelPropertiesSystem
void ModelPropertiesSystem::Run(GameWorld& world, std::unordered_map<ObjectID, Model*>& index)
{
    for(auto iter = index.begin(); iter != index.end(); ++iter) {

        auto& node = *iter->second;

        if(!node.Marked)
            continue;

        // TODO: this check could be for graphics outside this loop
        if(node.GraphicalObject) {
            node.ApplyMeshName();
            node.GraphicalObject->setMaterial(node.Material);
        }

        node.Marked = false;
    }
}

// ------------------------------------ //
// SendableSystem
//! \brief Helper for SendableSystem::HandleNode to not have as much code duplication for
//! client and server code
void SendableHandleHelper(ObjectID id, Sendable& obj, GameWorld& world,
    const std::shared_ptr<Connection>& connection,
    const std::shared_ptr<EntityState>& curstate, bool server)
{
    const auto ticknumber = world.GetTickNumber();

    // Determine if this is initial data or an update
    bool initial = true;

    for(auto iter = obj.UpdateReceivers.begin(); iter != obj.UpdateReceivers.end(); ++iter) {

        if(iter->CorrespondingConnection == connection) {

            // Updating an existing connection
            initial = false;

            // Prepare the update packet data  //
            sf::Packet updateData;

            // Do not use last confirmed if it is too old
            int referencetick = -1;

            if(iter->LastConfirmedData &&
                (ticknumber <
                    iter->LastConfirmedTickNumber + BASESENDABLE_STORED_RECEIVED_STATES - 1)) {
                referencetick = iter->LastConfirmedTickNumber;

                // Now calculate a delta update from curstate to the last confirmed state
                curstate->CreateUpdatePacket(*iter->LastConfirmedData, updateData);

            } else {

                // Data is too old (or doesn't exist) and cannot be used //
                curstate->AddDataToPacket(updateData);

                iter->LastConfirmedTickNumber = -1;
                iter->LastConfirmedData.reset();
            }

            // Send the update packet
            auto sentThing = connection->SendPacketToConnectionWithTrackingWithoutGuarantee(
                ResponseEntityUpdate(
                    0, world.GetID(), ticknumber, referencetick, id, std::move(updateData)));

            iter->AddSentPacket(ticknumber, curstate, sentThing);

            break;
        }
    }

    if(initial) {

        // Only server sends the static state, as the server doesn't want to receive that data,
        // because it was the one who initially sent it to any client that has local control
        if(server) {
            // First create a packet which will store the component initial data and lists all
            // the components
            // TODO: this data could be cached when an entity is created as it will be sent to
            // all the players
            sf::Packet initialComponentData;
            uint32_t componentCount = world.CaptureEntityStaticState(id, initialComponentData);

            // Send the initial response
            connection->SendPacketToConnection(
                std::make_shared<ResponseEntityCreation>(
                    0, world.GetID(), id, componentCount, std::move(initialComponentData)),
                RECEIVE_GUARANTEE::Critical);
        }

        // And then send the initial state packet
        sf::Packet updateData;

        curstate->AddDataToPacket(updateData);

        auto sentThing = connection->SendPacketToConnectionWithTrackingWithoutGuarantee(
            ResponseEntityUpdate(0, world.GetID(), ticknumber, -1, id, std::move(updateData)));

        // And add the connection to the receivers
        obj.UpdateReceivers.emplace_back(connection);
        obj.UpdateReceivers.back().AddSentPacket(ticknumber, curstate, sentThing);
    }
}

DLLEXPORT void SendableSystem::HandleNode(ObjectID id, Sendable& obj, GameWorld& world)
{
    const bool isServer = world.GetNetworkSettings().IsAuthoritative;

    const auto& players = world.GetConnectedPlayers();
    const auto& serverConnection = world.GetServerForLocalControl();

    // Create current state here as one or more connections should require it //
    // TODO: this could be removed entirely if we don't allow delta compression within a single
    // component. Other possible approach is to recycle old captured state objects
    auto curState = std::make_shared<EntityState>();

    world.CaptureEntityState(id, *curState);

    if(!curState) {

        LOG_ERROR(
            "SendableSystem: created invalid state for entity, id: " + Convert::ToString(id));
        return;
    }

    // Detect successful packets and closed connections
    for(auto iter = obj.UpdateReceivers.begin(); iter != obj.UpdateReceivers.end();) {
        // This is closed if the connection is invalid
        if(!iter->CorrespondingConnection->IsValidForSend()) {
            iter = obj.UpdateReceivers.erase(iter);
            continue;
        }

        bool exists = false;

        if(isServer) {
            // Or if the player has been removed from the world

            for(const auto& player : players) {

                if(iter->CorrespondingConnection == player->GetConnection()) {
                    exists = true;
                    break;
                }
            }
        } else {

            if(iter->CorrespondingConnection == serverConnection)
                exists = true;
        }

        if(!exists) {
            iter = obj.UpdateReceivers.erase(iter);
            continue;
        }

        // Check the most recent received state
        iter->CheckReceivedPackets();

        // Still valid, move to next
        ++iter;
    }

    // Handle sending updates
    if(isServer) {
        for(const auto& player : players) {

            const auto& connection = player->GetConnection();

            SendableHandleHelper(id, obj, world, connection, curState, isServer);
        }
    } else {

        if(serverConnection && serverConnection->IsValidForSend()) {

            SendableHandleHelper(id, obj, world, serverConnection, curState, isServer);
        }
    }
}
// ------------------------------------ //
// DLLEXPORT void ReceivedSystem::Run(
//     GameWorld& world, std::unordered_map<ObjectID, Received*>& Index)
// {
//     const float progress = world.GetTickProgress();
//     const auto tick = world.GetTickNumber();

//     for(auto iter = Index.begin(); iter != Index.end(); ++iter) {

//         auto& node = *iter->second;

//         // Unmarked nodes should have invalid interpolation status
//         if(!node.Marked)
//             return;

//         if(!node.LocallyControlled) {

//             // Interpolate received states //
//             float adjustedprogress = progress;
//             const Received::StoredState* first;
//             const Received::StoredState* second;

//             try {
//                 node.GetServerSentStates(&first, &second, tick, adjustedprogress);
//             } catch(const InvalidState&) {

//                 // If not found unmark to avoid running unneeded //
//                 node.Marked = false;
//                 continue;
//             }

//             DEBUG_BREAK;
//             // first->Interpolate(second, adjustedprogress);

//         } else {

//             // Send updates to the server //
//         }
//     }
// }
// ------------------------------------ //
// AnimationSystem
DLLEXPORT void AnimationSystem::Run(
    GameWorld& world, std::unordered_map<ObjectID, Animated*>& index, int tick, int timeintick)
{
    Graphics* graphics = Engine::Get()->GetGraphics();

    for(auto iter = index.begin(); iter != index.end(); ++iter) {

        Animated& animated = *iter->second;

        if(!animated.Animation)
            continue;

        if(animated.Marked) {
            animated.Marked = false;

            // Apply all properties and start playback
            for(size_t animationIndex = 0; animationIndex < animated.Animations.size();
                ++animationIndex) {

                auto& animation = animated.Animations[animationIndex];

                // Load clip
                if(!animation._LoadedAnimation || animation.NameMarked) {
                    animation._LoadedAnimation =
                        graphics->LoadAnimationClipByName(animation.Name);
                    animation.NameMarked = false;

                    if(!animation._LoadedAnimation) {
                        LOG_ERROR("AnimationSystem: failed to load animation named: " +
                                  animation.Name);
                        continue;
                    }
                }

                bs::AnimationClipState state;
                if(!animated.Animation->getState(animation._LoadedAnimation, state)) {

                    // Not playing currently. setState will begin playback, fill the extra info
                    // Is this needed?
                    state = bs::AnimationClipState();
                    // state.layer = static_cast<uint32_t>(animationIndex);
                    state.time = 0.f;
                }

                // TODO: if animations are removed the indexes need fixing up. Probably needs
                // to switch to a dynamic numbering scheme

                // getState doesn't return the correct layer so this is always applied
                state.layer = static_cast<uint32_t>(animationIndex);

                state.speed = animation.SpeedFactor;
                state.stopped = animation.Paused;
                state.wrapMode =
                    animation.Loop ? bs::AnimWrapMode::Loop : bs::AnimWrapMode::Clamp;

                animated.Animation->setState(animation._LoadedAnimation, state);
            }

            // Then disable other animations
            for(uint32_t i = 0; i < animated.Animation->getNumClips(); ++i) {

                bool found = false;
                auto clip = animated.Animation->getClip(i);

                for(const auto& animation : animated.Animations) {
                    if(animation._LoadedAnimation == clip) {
                        found = true;
                        break;
                    }
                }

                if(!found) {
                    LOG_WRITE("TODO: animation stopping for AnimationSystem");
                }
            }
        }
    }
}
