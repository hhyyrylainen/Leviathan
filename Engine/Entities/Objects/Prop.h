#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Entities/Bases/BaseObject.h"
#include "Entities/Bases/BaseRenderable.h"
#include "Entities/Bases/BasePositionable.h"
#include "Entities/Bases/BaseScalable.h"
#include "Entities/Bases/BasePhysicsObject.h"
#include "Entities/Bases/BaseConstraintable.h"
#include "Entities/Bases/BaseParentable.h"
#include "Entities/Bases/BaseSendableEntity.h"
#include "Events/CallableObject.h"

namespace Leviathan{ namespace Entity{
        
        //! \brief A movable model loaded from a file
        //! \todo Make sure that _MarkDataUpdated is called enough
        class Prop : public virtual BaseObject, public virtual BaseRenderable,
                       public BaseConstraintable, public BaseParentable,
                       public BaseSendableEntity, public BasePhysicsObject,
                       public virtual CallableObject
        {
            friend BaseSendableEntity;
        public:
            
            DLLEXPORT Prop(bool hidden, GameWorld* world);
            DLLEXPORT virtual ~Prop();
            
            DLLEXPORT bool Init(const std::string &modelfile);
            DLLEXPORT virtual void ReleaseData();

            static void PropPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix,
                int threadIndex);

            DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

            //! \copydoc BaseSendableEntity::CaptureState
            DLLEXPORT std::shared_ptr<ObjectDeltaStateData> CaptureState(Lock &guard, int tick)
                override;

            DLLEXPORT int OnEvent(Event** pEvent) override;
            DLLEXPORT int OnGenericEvent(GenericEvent** pevent) override;

            //! \copydoc BaseSendableEntity::CreateStateFromPacket
            DLLEXPORT virtual std::shared_ptr<ObjectDeltaStateData> CreateStateFromPacket(int tick,
                sf::Packet &packet) const override;

            REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(Prop);
            
        protected:
            //! \brief Constructs a prop for receiving through the network
            Prop(bool hidden, GameWorld* world, int netid);

            //! \copydoc BaseSendableEntity::_LoadOwnDataFromPacket
            virtual bool _LoadOwnDataFromPacket(sf::Packet &packet) override;

            //! \copydoc BaseSendableEntity::_SaveOwnDataToPacket
            virtual void _SaveOwnDataToPacket(sf::Packet &packet) override;

            //! \copydoc BaseConstraintable::_SendCreatedConstraint
            void _SendCreatedConstraint(BaseConstraintable* other, Entity::BaseConstraint* ptr) override;

            void _OnNewStateReceived() override;

            
            
            // for setting new values to graphical object and physical object //
            virtual void _UpdatePhysicsObjectLocation(Lock &guard) override;

            BaseConstraintable* BasePhysicsGetConstraintable() override;
            // ------------------------------------ //

            std::string ModelFile;

            bool ListeningForEvents;
        };

}}


