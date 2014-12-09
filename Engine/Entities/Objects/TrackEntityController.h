#ifndef LEVIATHAN_TRACKENTITYCONTROLLER
#define LEVIATHAN_TRACKENTITYCONTROLLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Bases/BaseEntityController.h"
#include "Entities/Bases/BaseSendableEntity.h"
#include "LocationNode.h"
#include "Events/CallableObject.h"
#include "Entities/Bases/BaseConstraintable.h"

#define TRACKCONTROLLER_DEFAULT_APPLYFORCE		12.f

namespace Leviathan{ namespace Entity{

        // Struct that can be used to tell this object to create nodes //
        struct TrackControllerPosition{
            TrackControllerPosition(){
            }
            TrackControllerPosition(const Float3 &pos, const Float4 &orientation) : Pos(pos), Orientation(orientation){
            }

            Float3 Pos;
            Float4 Orientation;
        };


        // This class is used to create movement paths for entities //
        class TrackEntityController : public BaseEntityController, public CallableObject, public BaseSendableEntity{
            
            friend BaseSendableEntity;
        public:
            
            DLLEXPORT TrackEntityController(GameWorld* world);
            DLLEXPORT virtual ~TrackEntityController();

            //! \brief Starts listening to events and verifies parameters
            //! \return Always true
            DLLEXPORT bool Init();
            DLLEXPORT virtual void ReleaseData();

            //! Events are used to receive when render is about to happen and update positions
            DLLEXPORT virtual int OnEvent(Event** pEvent);
        
            //! Does nothing
            DLLEXPORT virtual int OnGenericEvent(GenericEvent** pevent);

            //! \brief Directly sets the progress towards next node (if set to 1.f goes to next node)
            DLLEXPORT void SetProgressTowardsNextNode(float progress);
        
            //! \brief Gets the progress towards next node, if at 1.f then last node is reached
            DLLEXPORT float GetProgressTowardsNextNode(){
                return NodeProgress;
            }

            //! \brief Controls the speed at which the entity moves along the track (set to negative to go backwards
            //! and 0.f to stop)
            DLLEXPORT void SetTrackAdvanceSpeed(const float &speed);
            
            DLLEXPORT inline float GetTrackAdvanceSpeed(){
                return ChangeSpeed;
            }

            //! \brief Gets the position of the current node (or Float3(0) if no nodes exist)
            DLLEXPORT Float3 GetCurrentNodePosition();
        
            //! \brief Gets the position of the next node, or Float3(0) if the final node is reached.
            //! \exception Doesn't throw exceptions
            DLLEXPORT Float3 GetNextNodePosition();

            //! This function creates a new node to the world and ads it to the track of this object
            DLLEXPORT void AddLocationToTrack(const Float3 &pos, const Float4 &dir);

            DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

            //! \brief Updates the entity positions
            //! \note You probably don't have to manually call this
            DLLEXPORT virtual void UpdateControlledPositions(float timestep);

            //! \copydoc BaseSendableEntity::AddUpdateToPacket
            DLLEXPORT virtual void AddUpdateToPacket(sf::Packet &packet, ConnectionInfo* receiver) override;

            //! \copydoc BaseSendableEntity::LoadUpdateFromPacket
            DLLEXPORT virtual bool LoadUpdateFromPacket(sf::Packet &packet) override;


        protected:

            TrackEntityController(int netid, GameWorld* world);
            
            //! \brief Internal function for making all data valid
            //!
            //! Checks for invalid reached node and progress
            void _SanityCheckNodeProgress();
        
            //! \brief Updates the controlled object
            //! \todo apply rotation
            void _ApplyTrackPositioning(float timestep);

            //! Removes applied forces from objects
            void _OnConstraintUnlink(BaseConstraint* ptr) override;

            //! \copydoc BaseSendableEntity::_LoadOwnDataFromPacket
            virtual bool _LoadOwnDataFromPacket(sf::Packet &packet) override;

            //! \copydoc BaseSendableEntity::_SaveOwnDataToPacket
            virtual void _SaveOwnDataToPacket(sf::Packet &packet) override;

            // ------------------------------------ //
        
            //! Number of the node that has been reached
            int ReachedNode;

            //! Percentage between ReachedNode and next node
            //! 1.f being next node reached and progress reset to 0
            float NodeProgress;

            //! The speed at which the node progress changes
            float ChangeSpeed;

            //! The amount of speed/force used to move the entities towards the track position
            float ForceTowardsPoint;

            //! List of positions that form the track
            //! \note These are owned by the track
            std::vector<shared_ptr<LocationNode>> TrackNodes;

            //! Internal flag for determining if an update is needed
            bool RequiresUpdate;

        };

    }
}
#endif
