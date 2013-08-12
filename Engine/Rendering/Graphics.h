#ifndef LEVIATHAN_GRAPHS
#define LEVIATHAN_GRAPHS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "3DRenderer.h"
#include "ViewCamera.h"
#include "TextureManager.h"
#include "Light.h"
#include "TextRenderer.h"

#include "..\RenderAction.h"
#include "..\CameraPos.h"
#include "..\BaseRenderable.h"
#include "..\RenderBridge.h"

#include "RenderingPassInfo.h"
#include "..\BaseRenderableBufferContainer.h"

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

		DLLEXPORT void SetDescObjects(const DxRendConf &dx11) { Dconfig = dx11; };
		DLLEXPORT bool Init(Window* wind, const DxRendConf &conf);
		DLLEXPORT void Release();

		DLLEXPORT bool Resize(int newwidth, int newheight);

		DLLEXPORT bool Frame(int mspassed, ViewerCameraPos* camerapostouse, vector<BaseRenderable*> &objects);

		DLLEXPORT void SubmitRenderBridge(const shared_ptr<RenderBridge> &brdg);
		DLLEXPORT shared_ptr<RenderBridge> GetBridgeForGui(int actionid);


		DLLEXPORT bool RenderAutomatic(Rendering::BaseRenderableBufferContainer* torenderbuffers, ShaderRenderTask* shaderparameters);


		DLLEXPORT inline TextRenderer* GetTextRenderer(){
			return TextRender;
		}
		DLLEXPORT inline Dx11Renderer* GetRenderer(){
			return Drenderer;
		}
		DLLEXPORT inline TextureManager* GetTextureManager(){
			return TextureKeeper;
		}
		DLLEXPORT inline Rendering::ShaderManager* GetShader(){
			return Shaders;
		}
		DLLEXPORT inline Window* GetWindow(){
			return Wind;
		}
		RenderingLight* Light;

		DLLEXPORT static Graphics* Get();

	private:
		bool Render(int mspassed, vector<BaseRenderable*> &objects);

		void DrawRenderActions(RenderingPassInfo* pass);
		HRESULT Create3DRenderer(Window* wind);
		void PurgeGuiArray();
		// ------------------------ //
		bool Initialized;

		Window* Wind;
		DxRendConf Dconfig;

		
		Dx11Renderer* Drenderer;
		Rendering::ShaderManager* Shaders;
		TextureManager* TextureKeeper;
		ViewCamera* ActiveCamera;
		TextRenderer* TextRender;

		// save this value //
		int GuiSmooth;

		// 2d Gui rendering //
		vector<shared_ptr<RenderBridge>> GuiObjs;


		// static //
		static Graphics* _gadapter;
	};
}
#endif
