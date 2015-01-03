#ifndef LEVIATHAN_BASEPHYSICSOBJECT
#define LEVIATHAN_BASEPHYSICSOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Newton/PhysicalWorld.h"
#include "BasePositionable.h"
#include "BaseObject.h"
#include "Common/SFMLPackets.h"
#include "boost/function.hpp"


#define BASEPHYSICS_CUSTOMMESSAGE_DATA_CHECK	{if(entitycustommessagetype >= ENTITYCUSTOMMESSAGETYPE_ADDAPPLYFORCE && entitycustommessagetype <= ENTITYCUSTOMMESSAGETYPE_SETVELOCITY){if(BasePhysicsCustomMessage(entitycustommessagetype, dataptr)) return true;}}
#define BASEPHYSICS_CUSTOMMESSAGE_GET_CHECK		{if(tmprequest->RequestObjectPart == ENTITYDATA_REQUESTTYPE_NEWTONBODY){if(BasePhysicsCustomGetData(tmprequest)) return true;}}


namespace Leviathan{


    //! \brief Holder for information regarding a single force
    //! \note Avoid using external state in the get callback as networked resimulation might become inaccurate
    //! and players will have a bad experience
	class ApplyForceInfo{
	public:
        //! \note Pass NULL for name if not used, avoid passing empty strings
        //! \param name The name to assign. This will be deleted by a unique_ptr
		DLLEXPORT ApplyForceInfo(bool addmass,
            boost::function<Float3(ApplyForceInfo* instance, BasePhysicsObject* object)> getforce,
            wstring* name = NULL);
        
		DLLEXPORT ApplyForceInfo(ApplyForceInfo &other);
        DLLEXPORT ApplyForceInfo(ApplyForceInfo &&other);
		DLLEXPORT ~ApplyForceInfo();

		DLLEXPORT ApplyForceInfo& operator =(const ApplyForceInfo &other);

		//! Set a name when you don't want other non-named forces to override this
		unique_ptr<wstring> OptionalName;
        
		//! Whether to multiply the force by mass, makes acceleration constant with different masses
		bool MultiplyByMass;
        
        //! The callback which returns the force
        //! \todo Allow deleting this force from the callback
        boost::function<Float3(ApplyForceInfo* instance, BasePhysicsObject* object)> Callback;
	};

    //! \brief Can hold all data used by BasePhysicsObject
    struct BasePhysicsData{

        Float3 Velocity;
		Float3 Torque;
    };


    //! \brief Inherited by objects that have physical bodies
	class BasePhysicsObject : virtual public BasePositionable, virtual public BaseObject{
	public:
		DLLEXPORT BasePhysicsObject();
		DLLEXPORT virtual ~BasePhysicsObject();


		DLLEXPORT NewtonCollision* GetPhysicsCollision(){
			return Collision;
		}
		DLLEXPORT NewtonBody* GetPhysicsBody(){
			return Body;
		}

		// physical interaction //
		DLLEXPORT void GiveImpulse(const Float3 &deltaspeed, const Float3 &point = Float3(0));

		// Adds an apply force (and possibly overwrites old one). Note: the pointer is deleted by this object //
		DLLEXPORT void ApplyForce(ApplyForceInfo* pointertohandle);

		// Just removes an existing force, pass empty wstring to delete the default named force
		DLLEXPORT bool RemoveApplyForce(const wstring &name);

		// Sets the absolute velocity of the object //
		DLLEXPORT void SetBodyVelocity(const Float3 &velocities);

		// Gets the velocity of this object //
		DLLEXPORT Float3 GetBodyVelocity();

        //! \brief Gets the torque of the body (rotational velocity)
        DLLEXPORT Float3 GetBodyTorque();

        //! \brief Sets the torque of the body
        //! \see GetBodyTorque
        DLLEXPORT void SetBodyTorque(const Float3 &torque);

		// Physical material setting in wstring form for your convenience //
		DLLEXPORT bool SetPhysicalMaterial(const wstring &materialname);

		// Higher performance material set if you use it in batches and
        // you have fetched the material id from PhysicalMaterialManager
		DLLEXPORT void SetPhysicalMaterialID(int ID);

        //! \brief Sets the linear dampening which slows down the object
        //! \param factor The factor to set. Must be between 0.f and 1.f. Default is 0.1f
        //! \note This can be used to set the viscosity of the substance the object is in for example to mimic
        //! drag in water (this needs verification...)
        //!
        //! More on this in the Newton wiki here:
        //! http://newtondynamics.com/wiki/index.php5?title=NewtonBodySetLinearDamping
        DLLEXPORT void SetLinearDampening(float factor = 0.1f);


        //! \brief Sendable entity old state checking for basic physical objects
        DLLEXPORT void CheckOldPhysicalState(PositionablePhysicalDeltaState* servercasted,
            PositionablePhysicalDeltaState* ourcasted, int tick, BaseSendableEntity* assendable);

        //! \brief Returns the default material ID for the world to which this entity belongs
        DLLEXPORT int GetDefaultPhysicalMaterialID() const;

        
        //! \brief Serializes basic physical state to a packet if Body is set
        //! \return True when Body is set
        //!
        //! Adds things like velocity and angular velocity (rotation)
        DLLEXPORT bool AddPhysicalStateToPacket(sf::Packet &packet);

        //! \brief Loads and applies a physical state from a packet
        //! \return True when the packet was properly constructed and the state was set
        DLLEXPORT bool ApplyPhysicalStateFromPacket(sf::Packet &packet);

        //! \brief Applies physical state from holder object
        DLLEXPORT void ApplyPhysicalState(BasePhysicsData &data);

        //! \brief Loads a physical state from a packet
        DLLEXPORT static bool LoadPhysicalStateFromPacket(sf::Packet &packet, BasePhysicsData &fill);

		// default physics callbacks that are fine in most cases //
		// Don't forget to pass the user data as BaseObject if using these //
		static void ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat timestep, int threadIndex);
		static void DestroyBodyCallback(const NewtonBody* body);
        

	protected:
		virtual void _DestroyPhysicalBody();
		virtual void PosUpdated() override;
		virtual void OrientationUpdated() override;

		// this function should update physics object location or if Immovable set, directly graphical objects //
		virtual void _UpdatePhysicsObjectLocation(ObjectLock &guard) = 0;

		// Adds all applied forces together //
		Float3 _GatherApplyForces(const float &mass);

		bool BasePhysicsCustomMessage(int message, void* data);
		bool BasePhysicsCustomGetData(ObjectDataRequest* data);

        //! \brief Called before position is changed in resimulate, can be used to interpolate position
        virtual void OnBeforeResimulateStateChanged();
        
		// ------------------------------------ //
		NewtonCollision* Collision;
		NewtonBody* Body;

        //! The set physical material
        //! If none is set this defaults to -1
        //! The default material ID from GetDefaultPhysicalMaterialID might be applied
        int AppliedPhysicalMaterial;

		bool Immovable;
		bool ApplyGravity;

		// force applying needs to be stored until (or longer) the apply force and torque callback //
		std::list<shared_ptr<ApplyForceInfo>> ApplyForceList;
	};

}

#endif
