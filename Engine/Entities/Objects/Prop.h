#ifndef LEVIATHAN_ENTITY_PROP
#define LEVIATHAN_ENTITY_PROP
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities\Bases\BaseObject.h"
#include "Entities\Bases\BaseRenderable.h"
#include "Entities\Bases\BasePositionable.h"
#include "Entities\Bases\BaseScalable.h"

namespace Leviathan{ namespace Entity{
	

	class Prop : public BaseObject, public BaseRenderable, public BasePositionable/*, public BaseScalable*/{
	public:
		DLLEXPORT Prop(bool hidden);
		DLLEXPORT virtual ~Prop();
		
		DLLEXPORT bool Init(const wstring &modelfile, GameWorld* world);
		DLLEXPORT virtual void Release();

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
		// ------------------------ //
		GameWorld* LinkedToWorld;

		Ogre::Entity* GraphicalObject;
		Ogre::SceneNode* ObjectsNode;

		// physics //
		NewtonCollision* Collision;
		NewtonBody* Body;
	};

}}
#endif