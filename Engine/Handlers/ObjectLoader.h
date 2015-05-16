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
	public:
		DLLEXPORT ObjectLoader(Engine* engine);

		//! \brief Loads prop to a GameWorld
		DLLEXPORT ObjectID LoadPropToWorld(GameWorld* world, Lock &worldlock,
            const std::string &name, int materialid, const Position::PositionData &pos =
                {Float3(0), Float4::IdentityQuaternion()});

        DLLEXPORT ObjectID LoadPropToWorld(GameWorld* world, const std::string &name,
            int materialid, const Position::PositionData &pos =
                {Float3(0), Float4::IdentityQuaternion()})
        {
            GUARD_LOCK_OTHER(world);
            return LoadPropToWorld(world, guard, name, materialid, pos); 
        }
        
		//! \brief Creates a brush with physical component and sets mass
		//! \param mass The mass of the brush, use 0.f for static object
		DLLEXPORT ObjectID LoadBrushToWorld(GameWorld* world, Lock &worldlock,
            const std::string &material, const Float3 &size, const float &mass, int materialid,
            const Position::PositionData &pos = {Float3(0), Float4::IdentityQuaternion()});

        DLLEXPORT inline ObjectID LoadBrushToWorld(GameWorld* world, const std::string &material,
            const Float3 &size, const float &mass, int materialid,
            const Position::PositionData &pos = {Float3(0), Float4::IdentityQuaternion()})
        {

            GUARD_LOCK_OTHER(world);
            return LoadBrushToWorld(world, material, size, mass, materialid, pos);
        }

        //! \brief Creates a track controller to a world
        //!
        //! Track controller makes entities move along a track at specified speed
		DLLEXPORT ObjectID LoadTrackControllerToWorld(GameWorld* world, Lock &worldlock,
            std::vector<Position::PositionData> &initialtrack);

        DLLEXPORT inline ObjectID LoadTrackControllerToWorld(GameWorld* world,
            std::vector<Position::PositionData> &initialtrack)
        {

            GUARD_LOCK_OTHER(world);
            return LoadTrackControllerToWorld(world, guard, initialtrack);
        }

		//! \brief Creates a trail entity to a world
        //!
        //! Set the dynamic property if you want to update the properties later
		DLLEXPORT ObjectID LoadTrailToWorld(GameWorld* world, Lock &worldlock,
            const std::string &material, const Trail::Properties &properties,
            bool allowupdatelater,
            const Position::PositionData &pos = {Float3(0), Float4::IdentityQuaternion()});

        DLLEXPORT inline ObjectID LoadTrailToWorld(GameWorld* world, const std::string &material,
            const Trail::Properties &properties, bool allowupdatelater,
            const Position::PositionData &pos = {Float3(0), Float4::IdentityQuaternion()})
        {

            GUARD_LOCK_OTHER(world);
            return LoadTrailToWorld(world, guard, material, properties, allowupdatelater, pos);
        }
            

	private:
		Engine* m_Engine;
	};

}

