// ------------------------------------ //
#include "ComponentState.h"

using namespace Leviathan;
// ------------------------------------ //

// ------------------------------------ //
// EntityState
DLLEXPORT void EntityState::CreateUpdatePacket(EntityState& olderstate, sf::Packet& packet)
{
    // We need to match up the components from us to the old states
    const bool checkSameIndexFirst =
        olderstate.ComponentStates.size() == ComponentStates.size();

    for(size_t i = 0; i < ComponentStates.size(); ++i) {

        const auto& component = ComponentStates[i];

        BaseComponentState* olderMatchingState = nullptr;

        if(checkSameIndexFirst) {

            if(olderstate.ComponentStates[i]->ComponentType == component->ComponentType) {

                olderMatchingState = olderstate.ComponentStates[i].get();
            }
        }

        if(!olderMatchingState) {

            for(const auto& oldComponent : olderstate.ComponentStates) {

                if(oldComponent->ComponentType == component->ComponentType) {

                    olderMatchingState = oldComponent.get();
                    break;
                }
            }
        }

        component->AddDataToPacket(packet, olderMatchingState);
    }
}

DLLEXPORT void EntityState::AddDataToPacket(sf::Packet& packet)
{
    for(const auto& component : ComponentStates)
        component->AddDataToPacket(packet, nullptr);
}
