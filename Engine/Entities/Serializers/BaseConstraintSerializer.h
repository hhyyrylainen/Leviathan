#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Networking/NetworkResponse.h"
#include "../../Common/SFMLPackets.h"
#include "Entities/Objects/Constraints.h"

namespace Leviathan{


    //! \brief Base class for custom constraint serializers
    //!
    //! Also contains code for serializing default constraints
	class BaseConstraintSerializer{
	public:
		DLLEXPORT BaseConstraintSerializer();
        DLLEXPORT virtual ~BaseConstraintSerializer();

        //! \brief Returns true if this object can handle the type
        DLLEXPORT virtual bool CanHandleType(Entity::ENTITY_CONSTRAINT_TYPE type) const;


        //! \brief Serializes a constraint
        DLLEXPORT virtual std::shared_ptr<sf::Packet> SerializeConstraint(
            Entity::BaseConstraint* constraint, Entity::ENTITY_CONSTRAINT_TYPE &type);

        //! \brief Creates a constraint from a serialized form
        //! \param object1 The first object in the constraint, should always be valid
        //! \param object2 The second object in the constraint, this may be NULL
        //! \param create Whether to create or destroy the constraint
        //! \return True when the data was valid, false otherwise even if the type is correct
        DLLEXPORT virtual bool UnSerializeConstraint(Constraintable &object1,
            Constraintable &object2, Entity::ENTITY_CONSTRAINT_TYPE type, sf::Packet &packet,
            bool create = true);

        BaseConstraintSerializer(const BaseConstraintSerializer&) = delete;
	};

}

