#ifndef LEVIATHAN_BASEPHYSICSOBJECT
#define LEVIATHAN_BASEPHYSICSOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Newton.h"
#include "BasePositionable.h"
#include "BaseObject.h"


namespace Leviathan{

	// fill this to apply a force //
	class ApplyForceInfo{
	public:
		// Note: look at the class for what parameters do, and pass NULL for name if not used (avoid passing empty strings) //
		DLLEXPORT ApplyForceInfo(const Float3 &forces, bool addmass, bool persist = true, wstring* name = NULL);
		DLLEXPORT ~ApplyForceInfo();

		DLLEXPORT ApplyForceInfo& operator =(ApplyForceInfo &other);

		// set name when you don't want other non-named forces override this //
		unique_ptr<wstring> OptionalName;
		// set if you don't want to call apply force every frame //
		bool Persist;
		// whether to multiply the force by mass, to get same speed to all objects that are applied force to //
		bool MultiplyByMass;
		// finally the amount to apply to each direction //
		Float3 ForcesToApply;
	private:
		// don't want this constructor to be called
		DLLEXPORT ApplyForceInfo(const ApplyForceInfo& other);
	};


	class BasePhysicsObject : public BasePositionable, virtual public BaseObject{
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
		// Just removes an existing force, pass empty wstring to delete default named force //
		DLLEXPORT bool RemoveApplyForce(const wstring &name);
		// Sets the absolute velocity of the object //
		DLLEXPORT void SetBodyVelocity(const Float3 &velocities);

		// default physics callbacks that are fine in most cases //
		// Don't forget to pass the user data as base object if using these //
		static void ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat timestep, int threadIndex);
		static void DestroyBodyCallback(const NewtonBody* body);


	protected:
		virtual void _DestroyPhysicalBody();
		virtual void PosUpdated();
		virtual void OrientationUpdated();
		// this function should update physics object location or if Immovable set, directly graphical objects //
		virtual void _UpdatePhysicsObjectLocation() = 0;
		// Adds all applied forces together //
		Float3 _GatherApplyForces();
		// ------------------------------------ //
		NewtonCollision* Collision;
		NewtonBody* Body;

		bool Immovable;
		bool ApplyGravity;

		// force applying needs to be stored until (or longer) the apply force and torque callback //
		std::list<shared_ptr<ApplyForceInfo>> ApplyForceList;
	};

}


#endif