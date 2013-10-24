#ifndef LEVIATHAN_CONSTRAINTS
#define LEVIATHAN_CONSTRAINTS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

struct NewtonJoint;

namespace Leviathan{ 
	class GameWorld;
namespace Entity{
	
	class BaseContraintable;

	// base class for all different types of constraints to inherit //
	class BaseContraint{
	public:
		DLLEXPORT BaseContraint(GameWorld* world, BaseContraintable* parent, BaseContraintable* child);
		DLLEXPORT virtual ~BaseContraint();

		// actually creates the Newton joint, won't work without calling this //
		DLLEXPORT bool Init();
		// calls the destroy function //
		DLLEXPORT void Release();

		// Child class method that sets the internal parameters used in Init function //
		//DLLEXPORT BaseContraint* SetParameters()


		// function called when either one of constraint parts wants to disconnect, destroys the entire constraint //
		// ptr to either the parent or child is used to skip call to it (the destructor there is already running)
		DLLEXPORT void ConstraintPartUnlinkedDestroy(BaseContraintable* callinginstance);


	protected:
		// called to verify params before init proceeds //
		virtual bool _CheckParameters() = 0;
		virtual bool _CreateActualJoint() = 0;
		// ------------------------------------ //
		BaseContraintable* ParentObject;
		BaseContraintable* ChildObject;
		// world is direct ptr since all joints MUST be destroyed before the world is released //
		GameWorld* OwningWorld;
		NewtonJoint* Joint;
	};

	// Different types of constraints //


	class SliderConstraint : public BaseContraint{
	public:
		DLLEXPORT SliderConstraint(GameWorld* world, BaseContraintable* parent, BaseContraintable* child);
		DLLEXPORT virtual ~SliderConstraint();


		// Call this before init to set the right parameters //
		// Axis is normalized axis in global coordinates and defines the axis along this object can move //
		DLLEXPORT BaseContraint* SetParameters(const Float3 &slidingaxis);


	protected:
		virtual bool _CheckParameters();
		virtual bool _CreateActualJoint();
		// ------------------------------------ //

		// stored parameters //
		Float3 Axis;

	};




}}
#endif