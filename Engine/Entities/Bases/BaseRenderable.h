#ifndef LEVIATHAN_BASE_RENDERABLE
#define LEVIATHAN_BASE_RENDERABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "../GameWorld.h"

namespace Leviathan{

	class BaseRenderable{
	public:
		DLLEXPORT BaseRenderable(bool hidden);
		DLLEXPORT virtual ~BaseRenderable();

		DLLEXPORT inline bool IsHidden(){
			return Hidden;
		}

		DLLEXPORT void SetHiddenState(bool hidden);

		// Returns the Graphical object //
		DLLEXPORT virtual Ogre::Entity* GetOgreEntity();

		// Messing with the materials //

		// \todo add some more sophisticated methods //
		DLLEXPORT void SetDefaultSubDefaultPassDiffuse(const Float4 &newdiffuse) THROWS;

		DLLEXPORT void SetOgreMaterialName(const string &name);

	protected:

		virtual void _OnHiddenStateUpdated();
		// ------------------------------------ //
		bool Hidden;

		Ogre::Entity* GraphicalObject;
		Ogre::SceneNode* ObjectsNode;
	};

}
#endif
