#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
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

        //! \brief Registers a new serializer
        //! \param serializer The serializer to add, the pointer will be deleted by this
        DLLEXPORT void AddSerializer(BaseConstraintSerializer* serializer);
        
        //! \brief Finds a suitable serializer and creates a constraint
        //! \return True when the constraint is created
        //! \param object1 The first entity, must be valid
        //! \param object2 The second entity, if any, may be NULL
        //! \param packet Containing the custom data for the constraint
        //! \param create Specifies whether to create or destroy the constraint
        DLLEXPORT bool CreateConstraint(Constraintable &object1, Constraintable &object2,
            ENTITY_CONSTRAINT_TYPE type, sf::Packet &packet, bool create);

        //! \brief Puts the custom data of a constraint into a packet
        //! \note The constraint is locked before this call
        DLLEXPORT std::shared_ptr<sf::Packet> SerializeConstraintData(
            BaseConstraint* constraint);


        DLLEXPORT static ConstraintSerializerManager* Get();

	protected:


        //! The list of valid serializers
        std::vector<BaseConstraintSerializer*> Serializers;


        static ConstraintSerializerManager* Staticinstance;
	};

}


