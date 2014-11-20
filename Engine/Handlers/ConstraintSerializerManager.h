#pragma once
#ifndef LEVIATHAN_CONSTRAINTSERIALIZERMANAGER
#define LEVIATHAN_CONSTRAINTSERIALIZERMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Serializers/BaseConstraintSerializer.h"
#include "Networking/NetworkResponse.h"
#include "Common/ThreadSafe.h"

namespace Leviathan{

	class ConstraintSerializerManager : public ThreadSafe{
	public:
		DLLEXPORT ConstraintSerializerManager();
        DLLEXPORT ~ConstraintSerializerManager();

        //! \brief Creates the default serializers
        DLLEXPORT bool Init();

        //! \brief Destroys all Serializers
        DLLEXPORT void Release();


        //! \brief Finds a suitable serializer and creates a constraint
        //! \return True when the constraint is created
        //! \param object1 The first entity, must be valid
        //! \param object2 The second entity, if any, may be NULL
        //! \param packet Containing the custom data for the constraint
        //! \param create Specifies whether to create or destroy the constraint
        DLLEXPORT bool CreateConstraint(BaseObject* object1, BaseObject* object2, Entity::ENTITY_CONSTRAINT_TYPE
            type, sf::Packet &packet, bool create);

        //! \brief Puts the custom data of a constraint into a packet
        //! \note The constraint is locked before this call
        DLLEXPORT shared_ptr<sf::Packet> SerializeConstraintData(Entity::BaseConstraint* constraint);


        DLLEXPORT static ConstraintSerializerManager* Get();

	protected:


        //! The list of valid serializers
        std::vector<BaseConstraintSerializer*> Serializers;


        static ConstraintSerializerManager* Staticinstance;
	};

}
#endif






