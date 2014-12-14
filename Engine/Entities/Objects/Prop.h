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

namespace Leviathan{ namespace Entity{
	
        //! \brief A movable model loaded from a file
        //! \todo Make sure that _MarkDataUpdated is called enough
        class Prop : virtual public BaseObject, public BaseRenderable, public BaseConstraintable, public BaseParentable,
                       public BaseSendableEntity, public BasePhysicsObject
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
            DLLEXPORT virtual shared_ptr<ObjectDeltaStateData> CaptureState() override;

            //! \copydoc BaseSendableEntity::LoadUpdateFromPacket
            DLLEXPORT virtual bool LoadUpdateFromPacket(sf::Packet &packet) override;

            
        protected:
            //! \brief Constructs a prop for receiving through the network
            Prop(bool hidden, GameWorld* world, int netid);

            //! \copydoc BaseSendableEntity::_LoadOwnDataFromPacket
            virtual bool _LoadOwnDataFromPacket(sf::Packet &packet) override;

            //! \copydoc BaseSendableEntity::_SaveOwnDataToPacket
            virtual void _SaveOwnDataToPacket(sf::Packet &packet) override;

            
            // for setting new values to graphical object and physical object //
            void _UpdatePhysicsObjectLocation();
            // ------------------------------------ //

            wstring ModelFile;
        };

}}

#endif
