#ifndef LEVIATHAN_GRAPHS
#define LEVIATHAN_GRAPHS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "..\BaseRenderable.h"

#include "FileSystem.h"
#include "3DRenderer.h"
#include "ViewCamera.h"
#include "TextureManager.h"

#include "ShaderManager.h"
#include "Light.h"
#include "MultTextureShader.h"
#include "Bitmap.h"
#include "TextRenderer.h"
#include "..\RenderAction.h"
#include "ColorQuad.h"
#include "..\CameraPos.h"

#include "..\RenderBridge.h"
#include "RenderingPassInfo.h"

#define TEXTURE_INACTIVE_TIME		30000
#define TEXTURE_UNLOAD_TIME			300000

namespace Leviathan{

	class Graphics : public EngineComponent{
	public:
		DLLEXPORT Graphics();
		DLLEXPORT ~Graphics();
		DLLEXPORT void SetDescObjects(DxRendConf dx11) { Dconfig = dx11; };
		DLLEXPORT bool Init(Window* wind);

		DLLEXPORT bool Frame(int mspassed, ViewerCameraPos* camerapostouse, vector<BaseRenderable*> &objects);
		DLLEXPORT void Release();
		DLLEXPORT void CleanUpRenderActions();

		DLLEXPORT bool HasRenderer();
		DLLEXPORT void RecreateRenderer(Window* wind);

		DLLEXPORT bool Resize(int newwidth, int newheight);


		DLLEXPORT int CountTextRenderLength(wstring &text, wstring &font, float heightmod, bool IsAbsolute, bool TranslateSize = true);
		DLLEXPORT int GetTextRenderHeight(wstring &font, float heightmod, bool IsAbsolute, bool TranslateSize = true);

		DLLEXPORT static Graphics* Get();
		DLLEXPORT TextRenderer* GetTextRenderer();
		DLLEXPORT Dx11Renderer* GetRenderer();
		DLLEXPORT TextureManager* GetTextureManager();
		DLLEXPORT ShaderManager* GetShader();
		DLLEXPORT Window* GetWindow();

		DLLEXPORT void SubmitRenderBridge(const shared_ptr<RenderBridge> &brdg);
		DLLEXPORT shared_ptr<RenderBridge> GetBridgeForGui(int actionid);

		RenderingLight* Light;

	private:
		bool Render(int mspassed, vector<BaseRenderable*> &objects);

		void PurgeGuiArray();

		void DrawRenderActions(D3DXMATRIX WorldMatrix, D3DXMATRIX ViewMatrix, D3DXMATRIX OrthoMatrix);
		HRESULT Create3DRenderer(Window* wind);
		void Destroy3DRenderer();
		// ------------------------ //

		bool Initialized;
		bool DxRenderer;

		Window* Wind;


		DxRendConf Dconfig;

		

		Dx11Renderer* Drenderer;
		ShaderManager* Shaders;
		TextureManager* TextureKeeper;
		ViewCamera* ActiveCamera;
		TextRenderer* TextRender;

		// save this value //
		int GuiSmooth;

		// 2d Gui rendering //
		vector<shared_ptr<RenderBridge>> GuiObjs;


		// static //
		static Graphics* gadapter;
	};

















}
#endif
