#ifndef LEVIATHAN_RENDERER
#define LEVIATHAN_RENDERER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#ifndef LEVIATHAN_LOGGER
#include "Logger.h"
#endif
#include "Window.h"

namespace Leviathan{

#define LOAD_TEST_GRAPH
#define FEATURE_LEVELS		3
#define CHAIN_BUFFERS		2
#define SIMULTANEOUS_RENDER_TARGETS		1
// ----------------------------------------------------- //
	class DxRendConf{
	public:
		DLLEXPORT DxRendConf(){
			Windowed = true;
			VSync = true;
			ScreenDepth = 1000.0f;
			ScreenNear = 0.1f;
			DriverType = D3D_DRIVER_TYPE_HARDWARE;
			AntiAliasing = 2;
		}
		DLLEXPORT DxRendConf(bool windowed, bool vsync, float scrdepth, float scrnear, D3D_DRIVER_TYPE dtype, int antia){
			Windowed = windowed;
			VSync = vsync;
			ScreenDepth = scrdepth;
			ScreenNear = scrnear;
			DriverType = dtype;
			AntiAliasing = antia;
		}
			bool Windowed;
			bool VSync;
			float ScreenDepth;
			float ScreenNear;
			D3D_DRIVER_TYPE DriverType;
			int AntiAliasing;

	};

// ----------------------------------------------------- //
	class Dx11Renderer : public EngineComponent{
	public:
		DLLEXPORT Dx11Renderer();
		DLLEXPORT ~Dx11Renderer();

		DLLEXPORT HRESULT Init(Window* wind, DxRendConf conf);
		DLLEXPORT bool Resize();
		DLLEXPORT void Release();

		DLLEXPORT void BeginRender(Float4 ClearColor);
		DLLEXPORT void EndRender();

		DLLEXPORT ID3D11Device* GetDevice() { return Device; };
		DLLEXPORT ID3D11DeviceContext* GetDeviceContext() { return DeviceContext; };

		DLLEXPORT void GetProjectionMatrix(D3DXMATRIX& projectionMatrix);
		DLLEXPORT void GetWorldMatrix(D3DXMATRIX& worldMatrix);
		DLLEXPORT void GetOrthoMatrix(D3DXMATRIX& orthoMatrix);

		DLLEXPORT void GetVideoCardInfo(wstring& name, int& memory);


		DLLEXPORT void TurnZBufferOn();
		DLLEXPORT void TurnZBufferOff();

		DLLEXPORT bool IsInited(){return Initialized;};

		DLLEXPORT void TurnOnAlphaBlending();
		DLLEXPORT void TurnOffAlphaBlending();


	private:
		bool Initialized;
		Window* Wind;
		DxRendConf Configuration;

		// data //
		bool Vsync;
		int VideoCardMemory;
		wstring VideoCardDesc;
		D3D_FEATURE_LEVEL* CurrentFeatureLevel;
		int AntiAliasing;
		int SampleQuality;
		// devices //
		IDXGISwapChain* SwapChain;
		ID3D11Device* Device;
		ID3D11DeviceContext* DeviceContext;
		ID3D11RenderTargetView* RenderTargetView;
		ID3D11Texture2D* DepthStencilBuffer;
		ID3D11DepthStencilState* DepthStencilState;
		ID3D11DepthStencilView* DepthStencilView;
		ID3D11RasterizerState* RasterState;
		D3DXMATRIX ProjectionMatrix;
		D3DXMATRIX WorldMatrix;
		D3DXMATRIX OrthoMatrix;

		ID3D11DepthStencilState* DepthDisabledStencilState;

		ID3D11BlendState* AlphaEnableBlendingState;
		ID3D11BlendState* AlphaDisableBlendingState;

#ifdef _DEBUG
		ID3D11Debug* DDebug;
#endif


	};

}
#endif