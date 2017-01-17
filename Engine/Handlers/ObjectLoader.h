#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "../Entities/Components.h"
#include "../Entities/GameWorld.h"

namespace Leviathan{

	//! \brief Class to collect all entity creations to one class
    //! \note All created objects are added to the world so that they are broadcast on
    //! the network (if this is a server)
    //! \todo Allow objects to be created that will be sent to clients only after the caller has
    //! had the chance to create constraints etc.
	class ObjectLoader{
        friend EntitySerializer;
	public:

        ObjectLoader() = delete;

	};

}

