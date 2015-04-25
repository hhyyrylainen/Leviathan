#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
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
		DLLEXPORT void SetDefaultSubDefaultPassDiffuse(const Float4 &newdiffuse);

		DLLEXPORT void SetOgreMaterialName(const std::string &name);

        //! \todo Move to a new class
        DLLEXPORT void SetScale(const Float3 &scale);

	protected:

		virtual void _OnHiddenStateUpdated();
		// ------------------------------------ //
		bool Hidden;

		Ogre::Entity* GraphicalObject;
		Ogre::SceneNode* ObjectsNode;
	};

}

