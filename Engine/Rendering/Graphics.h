#ifndef LEVIATHAN_GRAPHICS
#define LEVIATHAN_GRAPHICS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Bases/BaseRenderable.h"

#include "Common/GraphicalInputEntity.h"
#include "Application/AppDefine.h"
#include "GUI/OverlayMaster.h"

namespace Leviathan{
	// forward declarations to avoid having tons of headers here that aren't necessary //
	namespace Rendering{
	class ShaderManager;
	class FontManager;
	}

	class TextureManager;

	class Graphics : public EngineComponent, Ogre::FrameListener{
	public:
		DLLEXPORT Graphics();
		DLLEXPORT ~Graphics();

		DLLEXPORT bool Init(AppDef* appdef);
		DLLEXPORT void Release();


		DLLEXPORT bool Frame();


		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

		DLLEXPORT inline TextureManager* GetTextureManager(){
			return TextureKeeper;
		}
		DLLEXPORT inline Rendering::FontManager* GetFontManager(){
			return Fonts;
		}
		DLLEXPORT inline Rendering::OverlayMaster* GetOverlayMaster(){
			return Overlays;
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
		bool InitializeOverlay();
		// ------------------------ //
		bool Initialized;

		AppDef* AppDefinition;
		TextureManager* TextureKeeper;

		// OGRE //
		unique_ptr<Ogre::Root> ORoot;
		Ogre::Log* OLog;
		Rendering::FontManager* Fonts;
		Rendering::OverlayMaster* Overlays;

		// static //
		static Graphics* Staticaccess;
	};
}
#endif
