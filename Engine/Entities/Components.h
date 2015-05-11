#pragma once

//! \file This file contains common components for entities

// ------------------------------------ //
#include "Include.h"
#include "../Common/Types.h"
#include "../Common/SFMLPackets.h"

#include "Component.h"
#include "CommonStateObjects.h"

namespace Leviathan{

    //! brief Class containing residue static helper functions
    class ComponentHelpers{

        ComponentHelpers() = delete;

    };

    //! \brief Entity has position and direction it is looking at
    //! \note Any possible locking needs to be handled by the caller
	class Position : public Component{
	public:
        
        //! \brief Can hold all data used by Position
        struct PositionData{

            Float3 _Position;
            Float4 _Orientation;
        };

    public:

        DLLEXPORT Position();

        //! \brief Initializes at specific position
        DLLEXPORT bool Init(const Float3 pos, const Float4 rot);

        //! \brief Initializes at 0, 0, 0
        DLLEXPORT bool Init();

        //! \brief Sets _Position and _Orientation to be the same as in the data
        DLLEXPORT void ApplyPositionData(const PositionData &data);

        //! \brief Adds data members to packet
        DLLEXPORT void AddDataToPacket(sf::Packet &packet) const;

        //! \brief Loads data members from packet
        //! \exception InvalidArgument When the packet format is invalid
        DLLEXPORT void ApplyDataFromPacket(sf::Packet &packet);
        
        //! \brief Loads member data to data
        //! \see ApplyPositionData
        DLLEXPORT static void LoadDataFromPacket(sf::Packet &packet, PositionData &data);

        //! \brief Interpolates the member variables between from and to based on progress
        DLLEXPORT void Interpolate(PositionDeltaState &from, PositionDeltaState &to,
            float progress);
        
        

        Float3 _Position;
        Float4 _Orientation;
	};

    //! \brief Entity has an Ogre scene node
    class RenderNode : public Component{
    public:

        DLLEXPORT RenderNode();

        //! \brief Initializes without any Node
        DLLEXPORT bool Init();

        //! \brief Gracefully releases while world is still valid
        DLLEXPORT void Release(Ogre::Scene* worldsscene);
        
        
        Ogre::SceneNode* Node;
    };

    //! \brief Entity is sendable to clients
    class Sendable : public Component{
    public:
        
        DLLEXPORT Sendable();

        //! \brief Inits sendable with specified type
        DLLEXPORT bool Init(SENDABLE_TYPE type);
        
        //! Type used to find the required components for sending
        SENDABLE_TYPE SendableHandleType;
    };

    
}
