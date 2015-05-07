#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Bases/BaseSendableEntity.h"


namespace Leviathan{

    //! Flags for which fields have changed
    //! \see PositionablePhysicalDeltaState
    enum PPDELTAUPDATED{

        // Position
        PPDELTAUPDATED_POS_X = 1 << 0,
        PPDELTAUPDATED_POS_Y = 1 << 1,
        PPDELTAUPDATED_POS_Z = 1 << 2,

        // Rotation
        PPDELTAUPDATED_ROT_X = 1 << 3,
        PPDELTAUPDATED_ROT_Y = 1 << 4,
        PPDELTAUPDATED_ROT_Z = 1 << 5,
        PPDELTAUPDATED_ROT_W = 1 << 6,

        // Velocity
        PPDELTAUPDATED_VEL_X = 1 << 7,
        PPDELTAUPDATED_VEL_Y = 1 << 8,
        PPDELTAUPDATED_VEL_Z = 1 << 9,

        // Torque
        PPDELTAUPDATED_TOR_X = 1 << 10,
        PPDELTAUPDATED_TOR_Y = 1 << 11,
        PPDELTAUPDATED_TOR_Z = 1 << 12
    };

    static const int16_t PPDELTA_ALL_UPDATED = PPDELTAUPDATED_POS_X | PPDELTAUPDATED_POS_Y | PPDELTAUPDATED_POS_Z |
                                    PPDELTAUPDATED_VEL_X | PPDELTAUPDATED_VEL_Y | PPDELTAUPDATED_VEL_Z |
                                    PPDELTAUPDATED_TOR_X | PPDELTAUPDATED_TOR_Y | PPDELTAUPDATED_TOR_Z |
                                    PPDELTAUPDATED_ROT_X | PPDELTAUPDATED_ROT_Y | PPDELTAUPDATED_ROT_Z |
                                    PPDELTAUPDATED_ROT_W;

    //! \brief Flags for positionable rotationable updated state
    //! \see PositionableRotationableDeltaState
    enum PRDELTAUPDATED{
        // Position
        PRDELTAUPDATED_POS_X = 1 << 0,
        PRDELTAUPDATED_POS_Y = 1 << 1,
        PRDELTAUPDATED_POS_Z = 1 << 2,

        // Rotation
        PRDELTAUPDATED_ROT_X = 1 << 3,
        PRDELTAUPDATED_ROT_Y = 1 << 4,
        PRDELTAUPDATED_ROT_Z = 1 << 5,
        PRDELTAUPDATED_ROT_W = 1 << 6
    };
    
    static const int8_t PRDELTA_ALL_UPDATED = PRDELTAUPDATED_POS_X | PRDELTAUPDATED_POS_Y | PRDELTAUPDATED_POS_Z |
                                    PRDELTAUPDATED_ROT_X | PRDELTAUPDATED_ROT_Y | PRDELTAUPDATED_ROT_Z |
                                    PRDELTAUPDATED_ROT_W;;

    //! \brief State object for entities that only have position and base physical components
	class PositionablePhysicalDeltaState : public ObjectDeltaStateData{
	public:
		DLLEXPORT PositionablePhysicalDeltaState(int tick, const Float3 &position, const Float4 &rotation,
            const Float3 &velocity, const Float3 &torque);
        
        //! \see CreateUpdatePacket
        DLLEXPORT PositionablePhysicalDeltaState(int tick, sf::Packet &packet);
        DLLEXPORT ~PositionablePhysicalDeltaState();

        //! \brief Templated creation function for all classes that inherit both BasePotitionable and
        //! BasePhysicsObject
        //! \param tick The world tick to place in the resulting state
        template<class CType>
        DLLEXPORT static std::unique_ptr<PositionablePhysicalDeltaState> CaptureState(
            CType &object, int tick)
        {

            return std::unique_ptr<PositionablePhysicalDeltaState>(
                new PositionablePhysicalDeltaState(tick, object.GetPos(), object.GetOrientation(),
                    object.GetBodyVelocity(), object.GetBodyTorque()));
        }

        //! \note The olderstate has to be of type PositionablePhysicalDeltaState
        DLLEXPORT virtual void CreateUpdatePacket(ObjectDeltaStateData* olderstate,
            sf::Packet &packet) override;

        DLLEXPORT bool FillMissingData(ObjectDeltaStateData &otherstate) override;

        Float3 Position;
        Float4 Rotation;
        Float3 Velocity;
        Float3 Torque;

        //! Only set on received versions, marks which fields are valid
        int16_t ValidFields;
	};

    //! \brief State for objects that only require location and rotation data to get to clients
    class PositionableRotationableDeltaState : public ObjectDeltaStateData{
    public:

        DLLEXPORT PositionableRotationableDeltaState(int tick, const Float3 &position,
            const Float4 &rotation);
        
        //! \see CreateUpdatePacket
        DLLEXPORT PositionableRotationableDeltaState(int tick, sf::Packet &packet);
        DLLEXPORT ~PositionableRotationableDeltaState();
        
        //! \brief Templated creation function for all classes that inherit BasePotitionable
        //! \param tick The world tick to place in the resulting state
        template<class CType>
        DLLEXPORT static std::unique_ptr<PositionableRotationableDeltaState> CaptureState(
            Lock &guard, CType &object, int tick)
        {

            return std::unique_ptr<PositionableRotationableDeltaState>(
                new PositionableRotationableDeltaState(tick, object.GetPos(guard),
                    object.GetOrientation(guard)));
        }

        //! \brief Automatically locking version of CaptureState
        template<class CType>
        DLLEXPORT static inline std::unique_ptr<PositionableRotationableDeltaState> CaptureState(
            CType &object, int tick)
        {
            GUARD_LOCK_OTHER((&object));
            return CaptureState<CType>(guard, object, tick);
        }

        //! \note The olderstate has to be of type PositionableRotationableDeltaState
        DLLEXPORT virtual void CreateUpdatePacket(ObjectDeltaStateData* olderstate,
            sf::Packet &packet) override;
        
        DLLEXPORT bool FillMissingData(ObjectDeltaStateData &otherstate) override;

        Float3 Position;
        Float4 Rotation;

        //! Only set on received versions, marks which fields are valid
        int8_t ValidFields;
    };

}

