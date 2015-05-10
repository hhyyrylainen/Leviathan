#pragma once

//! \file This file contains common components for entities

// ------------------------------------ //
#include "Include.h"
#include "../Common/Types.h"

#include "Component.h"


namespace Leviathan{

    //! \brief Entity has position and direction it is looking at
	class Position : public Component{
	public:

        Position();


        Float3 _Position;
        Float4 _Orientation;
	};

    //! \brief Entity has an Ogre scene node
    class RenderNode : public Component{
    public:

        
        Ogre::SceneNode* Node;
    };

    //! \brief Entity is sendable to clients
    class Sendable : public Component{
    public:

        
        //! Type used to find the required components for sending
        int SendableHandleType;
    };

    
}
