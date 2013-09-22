#ifndef LEVIATHAN_GRAPHICS
#define LEVIATHAN_GRAPHICS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities\ViewerCameraPos.h"
#include "TextureManager.h"


#include <OgreManualObject.h>
//#include <Terrain/OgreTerrain.h>
//#include <Terrain/OgreTerrainGroup.h>
#include <OgreFrameListener.h>

#include <Terrain/OgreTerrainMaterialGeneratorA.h>

#include "GUI\RenderAction.h"
#include "Entities\Bases\BaseRenderable.h"
#include "GUI\RenderBridge.h"

#include "Light.h"
#include "Common\Window.h"
#include "Application\AppDefine.h"
#include "WorldTerrain.h"

#define TEXTURE_INACTIVE_TIME		30000
#define TEXTURE_UNLOAD_TIME			300000

namespace Leviathan{
	// forward declarations to avoid having tons of headers here that aren't necessary //
	namespace Rendering{
	class ShaderManager;
	class OverlayMaster;
	class FontManager;
	}

	class Graphics : public EngineComponent, Ogre::FrameListener{
	public:
		DLLEXPORT Graphics();
		DLLEXPORT ~Graphics();

		DLLEXPORT bool Init(AppDef* appdef);
		DLLEXPORT void Release();


		DLLEXPORT bool Frame(int mspassed, ViewerCameraPos* camerapostouse, vector<BaseRenderable*> &objects);

		DLLEXPORT void SubmitRenderBridge(const shared_ptr<RenderBridge> &brdg);
		DLLEXPORT shared_ptr<RenderBridge> GetBridgeForGui(int actionid);


		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

		DLLEXPORT void SaveScreenShot(const string &filename);



		//DLLEXPORT bool RenderAutomatic(Rendering::BaseRenderableBufferContainer* torenderbuffers, ShaderRenderTask* shaderparameters);

		DLLEXPORT inline Window* GetOwningWindow(){
			return AppDefinition->GetWindow();
		}

		DLLEXPORT inline TextureManager* GetTextureManager(){
			return TextureKeeper;
		}
		DLLEXPORT inline Rendering::OverlayMaster* GetOverlayMaster(){
			return Overlays;
		}

		DLLEXPORT inline Rendering::FontManager* GetFontManager(){
			return Fonts;
		}
		DLLEXPORT inline AppDef* GetDefinitionObject(){
			return AppDefinition;
		}
		DLLEXPORT inline Ogre::SceneManager* GetWindowScene(){
			return MainScene;
		}

		RenderingLight* Light;

		DLLEXPORT static Graphics* Get();
	private:
		bool Render(int mspassed, vector<BaseRenderable*> &objects);

		void DrawRenderActions();
		bool InitializeOgre(AppDef* appdef);
		bool CreateDefaultRenderView();
		bool CreateCameraAndNodesForScene();
		void CreateTestObject();
		void PurgeGuiArray();
		void ConfigureTestRendering();
		bool InitializeOverlay();
		// ------------------------ //
		bool Initialized;

		AppDef* AppDefinition;
		TextureManager* TextureKeeper;

		// save this value //
		int GuiSmooth;

		// 2d Gui rendering //
		std::vector<shared_ptr<RenderBridge>> GuiObjs;


		// OGRE //
		unique_ptr<Ogre::Root> ORoot;
		Ogre::Log* OLog;
		Ogre::SceneManager* MainScene;
		Ogre::Camera* MainCamera;
		Ogre::SceneNode* MainCameraNode;
		Ogre::Viewport* MainViewport;

		Rendering::OverlayMaster* Overlays;
		Rendering::FontManager* Fonts;

		//WorldTerrain* Terrain;

		// static //
		static Graphics* Staticaccess;
	};
}
#endif
