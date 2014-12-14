#pragma once
#ifndef LEVIATHAN_COMMONSTATEOBJECTS
#define LEVIATHAN_COMMONSTATEOBJECTS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Bases/BaseSendableEntity.h"


namespace Leviathan{


    //! Flags for which fields have changed
    

    //! \brief State object for entities that only have position and base physical components
	class PositionablePhysicalDeltaState : public ObjectDeltaStateData{
	public:
		DLLEXPORT PositionablePhysicalDeltaState(const Float3 &position, const Float3 &velocity, const Float3 &torque);
        DLLEXPORT ~PositionablePhysicalDeltaState();

        //! \brief Templated creation function for all classes that inherit both BasePotitionable and
        //! BasePhysicsObject
        template<class CType>
        DLLEXPORT unique_ptr<PositionablePhysicalDeltaState> CaptureState(CType &object){

            return make_shared<PositionablePhysicalDeltaState>(object->GetPosition(), object->GetVelocity(),
                object->GetTorque());
        }

        //! \note The olderstate has to be of type PositionablePhysicalDeltaState
        DLLEXPORT virtual void CreateUpdatePacket(ObjectDeltaStateData* olderstate, sf::Packet &packet) override;

	protected:


        Float3 Position;
        Float3 Velocity;
        Float3 Torque;
	};

}
#endif
