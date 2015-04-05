#pragma once
#ifndef LEVIATHAN_ENTITY_PROP
#define LEVIATHAN_ENTITY_PROP
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Bases/BaseObject.h"
#include "Entities/Bases/BaseRenderable.h"
#include "Entities/Bases/BasePositionable.h"
#include "Entities/Bases/BaseScalable.h"
#include "Entities/Bases/BasePhysicsObject.h"
#include "Entities/Bases/BaseConstraintable.h"
#include "Entities/Bases/BaseParentable.h"
#include "Entities/Bases/BaseSendableEntity.h"
#ifdef NETWORK_USE_SNAPSHOTS
#include "Entities/Bases/BaseTimedInterpolated.h"
#include "Events/CallableObject.h"
#else
#include "Entities/Bases/BaseInterpolated.h"
#endif //NETWORK_USE_SNAPSHOTS

namespace Leviathan{ namespace Entity{
        
        //! \brief A movable model loaded from a file
        //! \todo Make sure that _MarkDataUpdated is called enough
        class Prop : public virtual BaseObject, public virtual BaseRenderable, public BaseConstraintable,
                       public BaseParentable, public BaseSendableEntity, public BasePhysicsObject,
#ifdef NETWORK_USE_SNAPSHOTS
                       public BaseTimedInterpolated, public virtual CallableObject
#else
                       public BaseInterpolated
#endif //NETWORK_USE_SNAPSHOTS
        {
            friend BaseSendableEntity;
        public:
            
            DLLEXPORT Prop(bool hidden, GameWorld* world);
            DLLEXPORT virtual ~Prop();
            
            DLLEXPORT bool Init(const wstring &modelfile);
            DLLEXPORT virtual void ReleaseData();

            static void PropPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix,
                int threadIndex);

            DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

            //! \copydoc BaseSendableEntity::CaptureState
            DLLEXPORT virtual shared_ptr<ObjectDeltaStateData> CaptureState(int tick) override;

#ifndef NETWORK_USE_SNAPSHOTS
            
            //! \copydoc BaseSendableEntity::VerifyOldState
            DLLEXPORT virtual void VerifyOldState(ObjectDeltaStateData* serversold,
                ObjectDeltaStateData* ourold, int tick) override;
#else
            DLLEXPORT int OnEvent(Event** pEvent) override;
            DLLEXPORT int OnGenericEvent(GenericEvent** pevent) override;

            DLLEXPORT bool SetStateToInterpolated(ObjectDeltaStateData &first, ObjectDeltaStateData &second,
                float progress);
#endif //NETWORK_USE_SNAPSHOTS

            //! \copydoc BaseSendableEntity::CreateStateFromPacket
            DLLEXPORT virtual shared_ptr<ObjectDeltaStateData> CreateStateFromPacket(int tick,
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

#ifdef NETWORK_USE_SNAPSHOTS
            void VerifySendableInterpolation() override;

            bool OnInterpolationFinished() override;

#else
            //! \copydoc BaseInterpolated::_GetCurrentActualPosition
            void _GetCurrentActualPosition(Float3 &pos) override;
        
            //! \copydoc BaseInterpolated::_GetCurrentActualRotation
            void _GetCurrentActualRotation(Float4 &rot) override;

            //! \copydoc BasePhysicsObject::OnBeforeResimulateStateChanged
            void OnBeforeResimulateStateChanged() override;

#endif //NETWORK_USE_SNAPSHOTS
            
            // for setting new values to graphical object and physical object //
            virtual void _UpdatePhysicsObjectLocation(ObjectLock &guard) override;

            BaseConstraintable* BasePhysicsGetConstraintable() override;
            // ------------------------------------ //

            wstring ModelFile;
        };

}}

#endif
