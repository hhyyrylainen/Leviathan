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

#include <Overlay\OgreOverlay.h>
#include <Overlay\OgreOverlayElement.h>
#include <Overlay\OgreOverlayManager.h>
#include <OgreManualObject.h>

#include "GUI\RenderAction.h"
#include "Entities\Bases\BaseRenderable.h"
#include "GUI\RenderBridge.h"

#include "Light.h"
#include "Common\Window.h"
#include "Application\AppDefine.h"

#define TEXTURE_INACTIVE_TIME		30000
#define TEXTURE_UNLOAD_TIME			300000

namespace Leviathan{
	// forward declarations to avoid having tons of headers here that aren't necessary //
	namespace Rendering{
	class ShaderManager;
	}

	class Graphics : public EngineComponent{
	public:
		DLLEXPORT Graphics();
		DLLEXPORT ~Graphics();

		DLLEXPORT bool Init(AppDef* appdef);
		DLLEXPORT void Release();


		DLLEXPORT bool Frame(int mspassed, ViewerCameraPos* camerapostouse, vector<BaseRenderable*> &objects);

		DLLEXPORT void SubmitRenderBridge(const shared_ptr<RenderBridge> &brdg);
		DLLEXPORT shared_ptr<RenderBridge> GetBridgeForGui(int actionid);


		//DLLEXPORT bool RenderAutomatic(Rendering::BaseRenderableBufferContainer* torenderbuffers, ShaderRenderTask* shaderparameters);

		DLLEXPORT inline TextureManager* GetTextureManager(){
			return TextureKeeper;
		}
		DLLEXPORT inline AppDef* GetDefinitionObject(){
			return AppDefinition;
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
		// ------------------------ //
		bool Initialized;

		AppDef* AppDefinition;
		TextureManager* TextureKeeper;

		// save this value //
		int GuiSmooth;

		// 2d Gui rendering //
		vector<shared_ptr<RenderBridge>> GuiObjs;


		// OGRE //
		unique_ptr<Ogre::Root> ORoot;
		Ogre::SceneManager* MainScene;
		Ogre::Camera* MainCamera;
		Ogre::SceneNode* MainCameraNode;
		Ogre::Viewport* MainViewport;

		// static //
		static Graphics* Staticaccess;
	};
}
#endif
