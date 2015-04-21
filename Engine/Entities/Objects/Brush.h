#ifndef LEVIATHAN_ENTITY_BRUSH
#define LEVIATHAN_ENTITY_BRUSH
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif

// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Bases/BaseObject.h"
#include "Entities/Bases/BaseRenderable.h"
#include "Entities/Bases/BasePositionable.h"
#include "Entities/Bases/BasePhysicsObject.h"
#include "Entities/Bases/BaseConstraintable.h"
#include "Entities/Bases/BaseParentable.h"
#include "Entities/Bases/BaseSendableEntity.h"
#include "Events/CallableObject.h"

namespace Leviathan{
	class GameWorld;
}

namespace Leviathan{ namespace Entity{

        //! \brief A (potentially) movable brush
        //! \todo Make sure that _MarkDataUpdated is called enough
        class Brush : public virtual BaseObject, public virtual BaseRenderable,
                        public BaseConstraintable, public BaseParentable,
                        public BaseSendableEntity, public BasePhysicsObject,
                        public virtual CallableObject
        {
            friend BaseSendableEntity;
        public:
            DLLEXPORT Brush(bool hidden, GameWorld* world);
            DLLEXPORT virtual ~Brush();

            DLLEXPORT virtual void ReleaseData();

            // different initialization functions for different box styles //
            // NOTE: leaving createphysics true creates a immovable box (uses mass = 0) //
            DLLEXPORT bool Init(const Float3 &dimensions, const string &material, bool createphysics = true);

            // call if you want to have this interact with other physical objects (set mass to 0 to be static) //
            DLLEXPORT void AddPhysicalObject(const float &mass = 0.f);


            DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

            static void BrushPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const
                matrix, int threadIndex);

            //! \copydoc BaseSendableEntity::CaptureState
            DLLEXPORT virtual shared_ptr<ObjectDeltaStateData> CaptureState(int tick) override;

            DLLEXPORT int OnEvent(Event** pEvent) override;
            DLLEXPORT int OnGenericEvent(GenericEvent** pevent) override;

            //! \copydoc BaseSendableEntity::CreateStateFromPacket
            DLLEXPORT virtual shared_ptr<ObjectDeltaStateData> CreateStateFromPacket(int tick,
                sf::Packet &packet) const override;

            REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(Brush);
            
        protected:

            //! \brief Constructs a brush for receiving through the network
            Brush(bool hidden, GameWorld* world, int netid);
            
            virtual void _UpdatePhysicsObjectLocation(ObjectLock &guard) override;

            //! \copydoc BaseSendableEntity::_LoadOwnDataFromPacket
            virtual bool _LoadOwnDataFromPacket(sf::Packet &packet) override;

            //! \copydoc BaseSendableEntity::_SaveOwnDataToPacket
            virtual void _SaveOwnDataToPacket(sf::Packet &packet) override;
            
            //! \copydoc BaseConstraintable::_SendCreatedConstraint
            void _SendCreatedConstraint(BaseConstraintable* other, Entity::BaseConstraint* ptr) override;

            void _OnNewStateReceived() override;
            
            BaseConstraintable* BasePhysicsGetConstraintable() override;            
            // ------------------------------------ //

            Ogre::ManualObject* BrushModel;

            string MeshName;
            string Material;
            float Mass;
            Float3 Sizes;

            bool ListeningForEvents;
        };

    }}

#endif
