#pragma once
#ifndef PONG_CONSTRAINTS
#define PONG_CONSTRAINTS
// ------------------------------------ //

// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Serializers/BaseConstraintSerializer.h"
#include "Entities/Objects/Constraints.h"
#include "Handlers/ConstraintSerializerManager.h"
#include "Entities/Bases/BaseConstraintable.h"

namespace Pong{

    enum PONG_CONSTRAINT_TYPE
    {
        PONG_CONSTRAINT_TYPE_EMOTIONAL = Leviathan::Entity::ENTITY_CONSTRAINT_TYPE_CUSTOM+1
    };


    enum LINK_TYPE
    {
        
        LINK_TYPE_PADDLE = 2,
        LINK_TYPE_TRACK,
        LINK_TYPE_GOAL,
        LINK_TYPE_LAST
    };

    class EmotionalConnection : public Leviathan::Entity::BaseConstraint{
    public:

        EmotionalConnection(Leviathan::GameWorld* world, Leviathan::BaseConstraintable* parent,
            Leviathan::BaseConstraintable* child);
        ~EmotionalConnection();

        EmotionalConnection* SetParameters(int plyid, LINK_TYPE whattolink);

        int GetPlayerNumber() const{

            return PlayerNumber;
        }

        LINK_TYPE GetType() const{

            return TypeToLink;
        }

    protected:

        virtual bool _CheckParameters() override;
        
        //! Links the player's paddle track to the PlayerSlot
        virtual bool _CreateActualJoint() override;
        // ------------------------------------ //
        
        //! The player number which is used to link the object to the right player
        int PlayerNumber;

        //! Defines which object needs to be linked to the player
        LINK_TYPE TypeToLink;
    };



    class PongConstraintSerializer : public Leviathan::BaseConstraintSerializer{
    public:

        PongConstraintSerializer();
        ~PongConstraintSerializer();


        //! \copydoc Leviathan::BaseConstraintSerializer::CanHandleType
        virtual bool CanHandleType(Leviathan::Entity::ENTITY_CONSTRAINT_TYPE type) const override;


        //! \copydoc Leviathan::BaseConstraintSerializer::SerializeConstraint
        virtual shared_ptr<sf::Packet> SerializeConstraint(Leviathan::Entity::BaseConstraint* constraint,
            Leviathan::Entity::ENTITY_CONSTRAINT_TYPE &type) override;

        //! \copydoc Leviathan::BaseConstraintSerializer::UnSerializeConstraint
        virtual bool UnSerializeConstraint(Leviathan::BaseObject* object1, Leviathan::BaseObject* object2,
            Leviathan::Entity::ENTITY_CONSTRAINT_TYPE type, sf::Packet &packet, bool create = true) override;


        static void Register(){

            Leviathan::ConstraintSerializerManager::Get()->AddSerializer(new PongConstraintSerializer());
        }
    };
    
    
}
#endif
