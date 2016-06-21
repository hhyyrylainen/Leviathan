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
        friend SendableEntitySerializer;
        
        ObjectLoader() = delete;
	public:
		//! \brief Loads prop to a GameWorld
		DLLEXPORT static ObjectID LoadPropToWorld(GameWorld* world, Lock &worldlock,
            const std::string &name, int materialid, const Position::PositionData &pos =
                {Float3(0), Float4::IdentityQuaternion()});

        DLLEXPORT static ObjectID LoadPropToWorld(GameWorld* world, const std::string &name,
            int materialid, const Position::PositionData &pos =
                {Float3(0), Float4::IdentityQuaternion()})
        {
            GUARD_LOCK_OTHER(world);
            return LoadPropToWorld(world, guard, name, materialid, pos); 
        }
        
		//! \brief Creates a brush with physical component and sets mass
		//! \param mass The mass of the brush, use 0.f for static object
		DLLEXPORT static ObjectID LoadBrushToWorld(GameWorld* world, Lock &worldlock,
            const std::string &material, const Float3 &size, const float &mass, int materialid,
            const Position::PositionData &pos = {Float3(0), Float4::IdentityQuaternion()});

        DLLEXPORT static inline ObjectID LoadBrushToWorld(GameWorld* world,
            const std::string &material, const Float3 &size, const float &mass, int materialid,
            const Position::PositionData &pos = {Float3(0), Float4::IdentityQuaternion()})
        {

            GUARD_LOCK_OTHER(world);
            return LoadBrushToWorld(world, guard, material, size, mass, materialid, pos);
        }

        //! \brief Creates a track controller to a world
        //!
        //! Track controller makes entities move along a track at specified speed
		DLLEXPORT static ObjectID LoadTrackControllerToWorld(GameWorld* world, Lock &worldlock,
            std::vector<Position::PositionData> &initialtrack);

        DLLEXPORT static inline ObjectID LoadTrackControllerToWorld(GameWorld* world,
            std::vector<Position::PositionData> &initialtrack)
        {

            GUARD_LOCK_OTHER(world);
            return LoadTrackControllerToWorld(world, guard, initialtrack);
        }

		//! \brief Creates a trail entity to a world
        //!
        //! Set the dynamic property if you want to update the properties later
		DLLEXPORT static ObjectID LoadTrailToWorld(GameWorld* world, Lock &worldlock,
            const std::string &material, const Trail::Properties &properties,
            bool allowupdatelater,
            const Position::PositionData &pos = {Float3(0), Float4::IdentityQuaternion()});

        DLLEXPORT static inline ObjectID LoadTrailToWorld(GameWorld* world,
            const std::string &material, const Trail::Properties &properties,
            bool allowupdatelater,
            const Position::PositionData &pos = {Float3(0), Float4::IdentityQuaternion()})
        {

            GUARD_LOCK_OTHER(world);
            return LoadTrailToWorld(world, guard, material, properties, allowupdatelater, pos);
        }

    protected:

        //! \see LoadBrushToWorld
        DLLEXPORT static bool LoadNetworkBrush(GameWorld* world, Lock &worldlock,
            ObjectID id, const std::string &material, const Float3 &size, const float &mass,
            int materialid, const Position::PositionData &pos, bool hidden);

        //! \see LoadPropToWorld
        DLLEXPORT static bool LoadNetworkProp(GameWorld* world, Lock &worldlock,
            ObjectID id, const std::string &modelfile, int materialid,
            const Position::PositionData &pos, bool hidden);

        //! \see LoadTrackControllerToWorld
        DLLEXPORT static bool LoadNetworkTrackController(GameWorld* world, Lock &worldlock,
            ObjectID id, size_t reachednode, float nodeprogress, float changespeed, float applyforce,
            const Parent::Data &childrendata, const PositionMarkerOwner::Data &positions);


    private:

        static void _CreateBrushModel(GameWorld* world, Lock &worldlock, ObjectID brush,
            Physics &physics, BoxGeometry &box, Position &position, float mass,
            const Float3 &size, bool hidden);

        static void _CreatePropCommon(GameWorld* world, Lock &worldlock,
            ObjectID prop, const std::string &ogrefile, Model &model, bool hidden);

        static void _CreatePropPhysics(GameWorld* world, Lock &worldlock, Model &model,
            Physics &physics, Position &position, ObjectFileList* proplist,
            const std::string &path, int materialid);
	};

}

