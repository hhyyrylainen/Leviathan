#ifndef LEVIATHAN_ENTITY_BRUSH
#define LEVIATHAN_ENTITY_BRUSH
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif

// ------------------------------------ //
// ---- includes ---- //
#include "..\Bases\BaseObject.h"
#include "..\Bases\BaseRenderable.h"
#include "..\Bases\BasePositionable.h"

namespace Leviathan{
	class GameWorld;
}

namespace Leviathan{ namespace Entity{


	class Brush : public BaseObject, public BaseRenderable, public BasePositionable{
	public:
		DLLEXPORT Brush(bool hidden);
		DLLEXPORT virtual ~Brush();

		DLLEXPORT virtual void Release();

		// different initialization functions for different box styles //
		// NOTE: leaving createphysics true creates a immovable box (uses mass = 0) //
		DLLEXPORT bool Init(GameWorld* world, const Float3 &dimensions, const string &material, bool createphysics = true);

		// call if you want to have this interact with other physical objects (set mass to 0 to be static) //
		DLLEXPORT void AddPhysicalObject(const float &mass = 0.f);

		DLLEXPORT virtual bool CheckRender(GraphicalInputEntity* graphics, int mspassed);

		// static movement update from physics //
		static void PropPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix, int threadIndex);
		static void ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat timestep, int threadIndex);
		static void DestroyBodyCallback(const NewtonBody* body);

	protected:
		// for setting new values to graphical object and physical object //
		virtual void PosUpdated();
		virtual void OrientationUpdated();
		void _UpdatePhysicsObjectLocation();
		void _DestroyPhysics();
		// ------------------------ //
		GameWorld* LinkedToWorld;

		string MeshName;
		Float3 Sizes;

		Ogre::Entity* GraphicalObject;
		Ogre::SceneNode* ObjectsNode;

		// physics //
		NewtonCollision* Collision;
		NewtonBody* Body;

		bool Immovable;
	};

}}
#endif