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
		// ------------------------------------ //
		NewtonCollision* Collision;
		NewtonBody* Body;

		bool Immovable;
		bool ApplyGravity;
	};

}


#endif