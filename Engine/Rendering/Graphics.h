#ifndef LEVIATHAN_GRAPHICS
#define LEVIATHAN_GRAPHICS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Bases/BaseRenderable.h"

#include "GraphicalInputEntity.h"
#include "Application/AppDefine.h"
#include "OgreFrameListener.h"

namespace Leviathan{
	// forward declarations to avoid having tons of headers here that aren't necessary //
	namespace Rendering{
	class ShaderManager;
	class FontManager;
	}

	class Graphics : public EngineComponent, Ogre::FrameListener{
	public:
		DLLEXPORT Graphics();
		DLLEXPORT ~Graphics();

		DLLEXPORT bool Init(AppDef* appdef);
		DLLEXPORT void Release();


		DLLEXPORT bool Frame();


		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

		DLLEXPORT inline Rendering::FontManager* GetFontManager(){
			return Fonts;
		}
		DLLEXPORT inline AppDef* GetDefinitionObject(){
			return AppDefinition;
		}
		DLLEXPORT inline Ogre::Root* GetOgreRoot(){
			return ORoot.get();
		}

		DLLEXPORT static Graphics* Get();
	private:

		bool InitializeOgre(AppDef* appdef);
		// ------------------------ //
		bool Initialized;

		AppDef* AppDefinition;

		// OGRE //
		unique_ptr<Ogre::Root> ORoot;
		Ogre::Log* OLog;
		Rendering::FontManager* Fonts;

		// static //
		static Graphics* Staticaccess;
	};
}
#endif
