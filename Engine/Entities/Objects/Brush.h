#ifndef LEVIATHAN_ENTITY_BRUSH
#define LEVIATHAN_ENTITY_BRUSH
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif

// ------------------------------------ //
// ---- includes ---- //
#include "../Bases/BaseObject.h"
#include "../Bases/BaseRenderable.h"
#include "../Bases/BasePositionable.h"
#include "../Bases/BasePhysicsObject.h"
#include "../Bases/BaseContraintable.h"
#include "../Bases/BaseParentable.h"
#include "../Bases/BaseSendableEntity.h"

namespace Leviathan{
	class GameWorld;
}

namespace Leviathan{ namespace Entity{


        class Brush : virtual public BaseObject, public BaseRenderable, public BaseContraintable,
                        public BaseParentable, public BaseSendableEntity
        {
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

            //! \copydoc BaseSendableEntity::AddUpdateToPacket
            DLLEXPORT virtual void AddUpdateToPacket(sf::Packet &packet, ConnectionInfo* receiver) override;

            //! \copydoc BaseSendableEntity::LoadUpdateFromPacket
            DLLEXPORT virtual bool LoadUpdateFromPacket(sf::Packet &packet) override;

        protected:
            
            virtual void _UpdatePhysicsObjectLocation();

            //! \copydoc BaseSendableEntity::_LoadOwnDataFromPacket
            virtual bool _LoadOwnDataFromPacket(sf::Packet &packet) override;

            //! \copydoc BaseSendableEntity::_SaveOwnDataToPacket
            virtual void _SaveOwnDataToPacket(sf::Packet &packet) override;
            
            // ------------------------------------ //

            Ogre::ManualObject* BrushModel;

            string MeshName;
            string Material;
            Float3 Sizes;
        };

    }}

#endif
