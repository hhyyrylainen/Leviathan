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
#include "..\Bases\BasePhysicsObject.h"
#include "..\Bases\BaseContraintable.h"
#include "..\Bases\BaseParentable.h"

namespace Leviathan{
	class GameWorld;
}

#pragma warning(push)
#pragma warning(disable : 4250)

namespace Leviathan{ namespace Entity{


	class Brush : virtual public BaseObject, public BaseRenderable, public BaseContraintable, public BaseParentable{
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

		static void BrushPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix, int threadIndex);

	protected:
		virtual void _UpdatePhysicsObjectLocation();
		// ------------------------------------ //
		string MeshName;
		Float3 Sizes;
	};

}}

#pragma warning(pop)

#endif