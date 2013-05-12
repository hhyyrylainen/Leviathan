#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERER
#include "3DRenderer.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include <D3D11.h>
//---------------------------------
Dx11Renderer::Dx11Renderer(){
	// init all to NULL //
	Initialized = false;
	CurrentFeatureLevel = NULL;
	SwapChain = NULL;
	Device = NULL;
	DeviceContext = NULL;
	RenderTargetView = NULL;
	DepthStencilBuffer = NULL;
	DepthStencilState = NULL;
	DepthStencilView = NULL;
	RasterState = NULL;

	DepthDisabledStencilState = NULL;

	AlphaEnableBlendingState = NULL;
	AlphaDisableBlendingState = NULL;

	Wind = NULL;

}
Dx11Renderer::~Dx11Renderer(){
	if(Initialized){
		Release();
	}

}
//---------------------------------
HRESULT Dx11Renderer::Init(Window* wind, DxRendConf conf){
	HRESULT hr = S_OK;

	// store window for later usage //
	Wind = wind;
	Configuration = conf;

	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	//DXGI_SWAP_CHAIN_DESC swapChainDesc;
	//D3D_FEATURE_LEVEL featureLevel;

	//float fieldOfView, screenAspect;

	Vsync = conf.VSync;
	AntiAliasing = conf.AntiAliasing;
	SampleQuality = 0;
	AntiAliasing = 1;

	// Create DirectX graphics interface factory.
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if(FAILED(hr)){
		Logger::Get()->Error(L"failed to create DirectX interface factory",0);
		return hr;
	}

	// create adapter from interface factory
	hr = factory->EnumAdapters(0, &adapter);
	if(FAILED(hr)){
		Logger::Get()->Error(L"failed to get graphics adapter",0);
		return hr;
	}

	// Enumerate the primary adapter
	hr = adapter->EnumOutputs(0, &adapterOutput);
	if(FAILED(hr)){
		Logger::Get()->Error(L"failed to enum adapter output",0);
		return hr;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if(FAILED(hr)){
		Logger::Get()->Error(L"failed to get graphics adapter",0);
		return hr;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if(!displayModeList){
		Logger::Get()->Error(L"empty display mode list",0);
		return hr;
	}

	// Now fill the display mode list structures.
	hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if(FAILED(hr)){
		Logger::Get()->Error(L"failed to fill display mode list structures",0);
		return hr;
	}
	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for(i=0; i<numModes; i++){
		if(displayModeList[i].Width == (unsigned int)wind->GetWidth()){
			if(displayModeList[i].Height == (unsigned int)wind->GetHeight()){
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get adapter description.
	hr = adapter->GetDesc(&adapterDesc);
	if(FAILED(hr)){
		Logger::Get()->Error(L"failed to get adapter description",0);
		return hr;
	}
	// get video card memory in mbits
	VideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// store card name
	VideoCardDesc = adapterDesc.Description;


	SAFE_DELETE_ARRAY(displayModeList);
	SAFE_RELEASE(adapterOutput);
	SAFE_RELEASE(adapter);
	SAFE_RELEASE(factory);

	//SAFE_RELEASE(factory); release after making swap chain

	// feature flags //
	D3D_FEATURE_LEVEL FeatureLevels[FEATURE_LEVELS];

	FeatureLevels[0] = D3D_FEATURE_LEVEL_11_0;
	FeatureLevels[1] = D3D_FEATURE_LEVEL_10_1;
	FeatureLevels[2] = D3D_FEATURE_LEVEL_10_0;

	// creation flags
	UINT CreateDeviceFlags = 0;
#ifdef _DEBUG
	CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// create device to check is modes supported //
	hr = D3D11CreateDevice(NULL,
		conf.DriverType,
		NULL,
		CreateDeviceFlags,
		FeatureLevels,
		FEATURE_LEVELS,
		D3D11_SDK_VERSION,
		&Device,
		CurrentFeatureLevel,
		&DeviceContext);
	if(FAILED(hr)){

		Logger::Get()->Error(L"D3D11Device creation failed", (int)hr);
#ifdef _DEBUG
		Logger::Get()->Info(L"This error can be caused by not having proper directx debug runtime installed, try reinstalling directx sdk (june 2010)");
#endif
		return hr;
	}

	// check does MSAA quality function //
	UINT QualityLevels = NULL;

	if(Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, AntiAliasing,  &QualityLevels) != S_OK){
		Logger::Get()->Error(L"D3D11Device: could not retrieve available quality levels",hr);
		return hr;
	}
	if(QualityLevels < 1){
		Logger::Get()->Error(L"D3D11Device: MSAA VALUE NOT SUPPORTED PLEASE TRY ANOTHER VALUE (1 WILL ALWAYS WORK)",QualityLevels);
		return hr;
	}

	SampleQuality = QualityLevels;


	// buffer desc //
	DXGI_MODE_DESC BufferDesc;
	ZeroMemory( &BufferDesc, sizeof( BufferDesc ) );


	BufferDesc.Width = wind->GetWidth();
	BufferDesc.Height = wind->GetHeight();
	if(conf.VSync){
		BufferDesc.RefreshRate.Numerator = 60;
		BufferDesc.RefreshRate.Denominator = 1;
	} else {
		BufferDesc.RefreshRate.Numerator = 0;
		BufferDesc.RefreshRate.Denominator = 1;
	}
	BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// sampling //
	DXGI_SAMPLE_DESC SampleDesc;
	ZeroMemory( &SampleDesc, sizeof( SampleDesc ) );

	// swap desc //
	DXGI_SWAP_CHAIN_DESC SwapDesc;
	ZeroMemory( &SwapDesc, sizeof( SwapDesc ) );

	// specify this here, after it's valid //
	if(AntiAliasing != 1){
		SampleDesc.Count = AntiAliasing;
		SampleDesc.Quality = SampleQuality-1;
	} else {
		SampleDesc.Count = 1;
		SampleDesc.Quality = 0;
	}

	SwapDesc.BufferCount = 2;
	SwapDesc.BufferDesc = BufferDesc;
	SwapDesc.SampleDesc = SampleDesc;
	SwapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapDesc.BufferCount = CHAIN_BUFFERS;
	SwapDesc.OutputWindow = wind->GetHandle();
	SwapDesc.Windowed = conf.Windowed;
	SwapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapDesc.Flags = NULL;

	//hr = D3D11CreateDeviceAndSwapChain(
	//	NULL,
	//	conf.DriverType,
	//	NULL,
	//	CreateDeviceFlags,
	//	FeatureLevels,
	//	FEATURE_LEVELS,
	//	D3D11_SDK_VERSION,
	//	&SwapDesc,
	//	&SwapChain,
	//	&Device,
	//	CurrentFeatureLevel,
	//	&DeviceContext);
	//if(!SUCCEEDED(hr)){
	//	Logger::Get()->Error(L"D3D11Device&swapchain creation failed",hr);
	//	return hr;
	//}
	
	//hr = factory->CreateSwapChain(
	//	Device,
	//	&SwapDesc,
	//	&SwapChain);
	//if(!SUCCEEDED(hr)){
	//	Logger::Get()->Error(L"SwapChain creation failed",hr);
	//	return hr;
	//}

	// get factory through device pointer //
	IDXGIDevice* dxgiDevice = 0;
	hr = Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	if(FAILED(hr)){

		QUICK_ERROR_MESSAGE;
		return hr;
	}

	IDXGIAdapter* tempdapter = 0;
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&tempdapter);
	if(FAILED(hr)){

		QUICK_ERROR_MESSAGE;
		return hr;
	}

	IDXGIFactory* devicesfactory = 0;
	hr = tempdapter->GetParent(__uuidof(IDXGIFactory), (void**)&devicesfactory);
	if(FAILED(hr)){

		QUICK_ERROR_MESSAGE;
		return hr;
	}


	// create swapchain from device's factory //
	hr = devicesfactory->CreateSwapChain(Device, &SwapDesc, &SwapChain);

	// release temporaries //
	SAFE_RELEASE(dxgiDevice);
	SAFE_RELEASE(tempdapter);
	SAFE_RELEASE(devicesfactory);

	if(!SwapChain){

		Logger::Get()->Error(L"3DRenderer: Failed to create swap chain", hr, true);
		return hr;
	}

	// finally get debug interface //
#ifdef _DEBUG
	hr = Device->QueryInterface(_uuidof(ID3D11Debug), (void **)&DDebug);
	if(!SUCCEEDED(hr)){
		Logger::Get()->Error(L"failed to get debug interface, continuing...");
		//return hr;
	}

#endif

	// some extra objects //

	// zero description //
	D3D11_RASTERIZER_DESC RasterDesc;
	ZeroMemory(&RasterDesc, sizeof(RasterDesc));


	// set up rasterizer state
	RasterDesc.AntialiasedLineEnable = false;
	RasterDesc.CullMode = D3D11_CULL_BACK;
	RasterDesc.DepthBias = 0;
	RasterDesc.DepthBiasClamp = 0.0f;
	RasterDesc.DepthClipEnable = true;
	RasterDesc.FillMode = D3D11_FILL_SOLID;
	RasterDesc.FrontCounterClockwise = false;
	RasterDesc.MultisampleEnable = AntiAliasing != 1;
	RasterDesc.ScissorEnable = false;
	RasterDesc.SlopeScaledDepthBias = 0.0f;

	// create rasterizer
	hr = Device->CreateRasterizerState(&RasterDesc, &RasterState);
	if(FAILED(hr)){
		Logger::Get()->Error(L"3DRenderer: Resize: failed to create rasterizer state",0);
		return hr;
	}

	// set rasterizer
	DeviceContext->RSSetState(RasterState);

	// initialize stencil state
	D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
	ZeroMemory(&DepthStencilDesc, sizeof(DepthStencilDesc));

	// set up description of the stencil state
	DepthStencilDesc.DepthEnable = true;
	DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	DepthStencilDesc.StencilEnable = true;
	DepthStencilDesc.StencilReadMask = 0xFF;
	DepthStencilDesc.StencilWriteMask = 0xFF;

	// stencil if pixel is front-facing
	DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// stencil if pixel is back-facing
	DepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	DepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// create depth stencil state
	hr = Device->CreateDepthStencilState(&DepthStencilDesc, &DepthStencilState);
	if(FAILED(hr)){

		Logger::Get()->Error(L"3DRenderer: Resize: failed to create depth stencil state",0);
		return hr;
	}

	// set depth stencil state
	DeviceContext->OMSetDepthStencilState(DepthStencilState, 1);

	// blend state descriptions //
	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	hr = Device->CreateBlendState(&blendStateDescription, &AlphaEnableBlendingState);
	if(FAILED(hr)){
		Logger::Get()->Error(L"3DRenderer: failed to create blending state with alpha blending enabled");
		return hr;
	}

	// Modify the description to create an alpha disabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;

	// Create the blend state using the description.
	hr = Device->CreateBlendState(&blendStateDescription, &AlphaDisableBlendingState);
	if(FAILED(hr)){

		Logger::Get()->Error(L"3DRenderer: failed to create blending state with alpha blending disabled");
		return hr;
	}

	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	// Clear the second depth stencil state before setting the parameters.
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

	// create it with depth set to 0 //
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the state using the device.
	hr = Device->CreateDepthStencilState(&depthDisabledStencilDesc, &DepthDisabledStencilState);
	if(FAILED(hr)){
		Logger::Get()->Error(L"3DRenderer: Resize: failed to create DepthStencilstate with depth disabled ");
		return hr;
	}

	// following initialization steps are required on resize so just call that here //
	if(!this->Resize()){
		// initial resizing failed, bad thing //

		Logger::Get()->Error(L"3DRenderer: Initial resizing failed, cannot continue", true);
		return hr;
	}
	

	Initialized = true;
	return hr;
}

bool Dx11Renderer::Resize(){
	HRESULT hr = S_OK;


	// release old objects if they exist //
	SAFE_RELEASE(RenderTargetView);
	SAFE_RELEASE(DepthStencilView);
	//SAFE_RELEASE(DepthDisabledStencilState);
	SAFE_RELEASE(DepthStencilBuffer);
	//SAFE_RELEASE(DepthStencilState);
	//SAFE_RELEASE(AlphaEnableBlendingState);
	//SAFE_RELEASE(AlphaDisableBlendingState);


	// resize back buffer //
	hr = SwapChain->ResizeBuffers(2, Wind->GetWidth(), Wind->GetHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, NULL);

	// get back buffer //
	ID3D11Texture2D* BackBuffer = NULL;
	hr = SwapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), (void **)&BackBuffer);
	if(!SUCCEEDED(hr)){
		Logger::Get()->Error(L"failed to get swap chain buffer",0);
		return false;
	}

	// create the render target view with back buffer pointer.
	hr = Device->CreateRenderTargetView(BackBuffer, NULL, &RenderTargetView);
	if(FAILED(hr)){

		Logger::Get()->Error(L"3DRenderer: Resize: failed to create render target view",0);
		return false;
	}

	// Release pointer to the back buffer as we no longer need it
	SAFE_RELEASE(BackBuffer);

	


	// initialize depth buffer desc
	D3D11_TEXTURE2D_DESC DepthBufferDesc;
	ZeroMemory(&DepthBufferDesc, sizeof(DepthBufferDesc));

	// Set up the description of the depth buffer
	DepthBufferDesc.Width = Wind->GetWidth();
	DepthBufferDesc.Height = Wind->GetHeight();
	DepthBufferDesc.MipLevels = 1;
	DepthBufferDesc.ArraySize = 1;
	DepthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//depthBufferDesc.SampleDesc.Count = AntiAliasing;
	//depthBufferDesc.SampleDesc.Quality = (SampleQuality==0?D3D11_DSV_DIMENSION_TEXTURE2D:D3D11_DSV_DIMENSION_TEXTURE2DMS);
	// setting anti aliasing //
	if(AntiAliasing != 1){
		DepthBufferDesc.SampleDesc.Count = AntiAliasing;
		DepthBufferDesc.SampleDesc.Quality = SampleQuality-1;
	} else {
		DepthBufferDesc.SampleDesc.Count = 1;
		DepthBufferDesc.SampleDesc.Quality = 0;
	}
	DepthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthBufferDesc.CPUAccessFlags = NULL;
	DepthBufferDesc.MiscFlags = NULL;

	// create texture for depth buffer
	hr = Device->CreateTexture2D(&DepthBufferDesc, NULL, &DepthStencilBuffer);
	if(FAILED(hr)){

		Logger::Get()->Error(L"3DRenderer: Resize: failed to create texture for depth buffer",0);
		return false;
	}
	



	
	// init depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// set up depth stencil view desc
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// create depth stencil view
	hr = Device->CreateDepthStencilView(DepthStencilBuffer, &depthStencilViewDesc, &DepthStencilView);
	if(FAILED(hr)){

		Logger::Get()->Error(L"3DRenderer: Resize: failed to create depth stencil view",0);
		return false;
	}

	// bind the render target and depth stencil to pipeline
	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);



	// setup viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(viewport));

	viewport.Width = (float)Wind->GetWidth();
	viewport.Height = (float)Wind->GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	DeviceContext->RSSetViewports(1, &viewport);

	// Setup the projection matrix.
	float fieldOfView = (float)D3DX_PI / 4.0f;
	float screenAspect = Wind->GetAspectRatio();

	// Create the projection matrix for 3D rendering.
	D3DXMatrixPerspectiveFovLH(&ProjectionMatrix, fieldOfView, screenAspect, Configuration.ScreenNear, Configuration.ScreenDepth);

	// Initialize the world matrix to the identity matrix.
	D3DXMatrixIdentity(&WorldMatrix);

	// Create an orthographic projection matrix for 2D rendering.
	D3DXMatrixOrthoLH(&OrthoMatrix, (float)Wind->GetWidth(), (float)Wind->GetHeight(), Configuration.ScreenNear, Configuration.ScreenDepth);	


	return true;
}

void Dx11Renderer::Release(){
	if(Initialized){
		if(SwapChain)
		{
			SwapChain->SetFullscreenState(false, NULL);
		}
		DeviceContext->ClearState();
		DeviceContext->Flush();

		SAFE_RELEASE(AlphaEnableBlendingState);
		SAFE_RELEASE(AlphaDisableBlendingState);

		SAFE_RELEASE(RasterState);

		SAFE_RELEASE(DepthStencilView);
		SAFE_RELEASE(DepthStencilState);
		SAFE_RELEASE(DepthStencilBuffer);
		SAFE_RELEASE(RenderTargetView);

		SAFE_RELEASE(DepthDisabledStencilState);

#ifdef _DEBUG
		SAFE_RELEASE(DDebug);
#endif

		SAFE_RELEASE(DeviceContext);
		SAFE_RELEASE(Device);
		SAFE_RELEASE(SwapChain);

		CurrentFeatureLevel = NULL;
		// discard card data //
		this->Initialized = false;
	}
}
//---------------------------------
void Dx11Renderer::BeginRender(Float4 ClearColor){

	//Float4 ClearColor(0.4f, 0.7f, 0.85f, 1);

	//ID3D11RenderTargetView* RenderTargetViews[SIMULTANEOUS_RENDER_TARGETS] = {pView};
	//ID3D11DepthStencilView* DepthTargetView = pDepthView;

	//DContext->OMSetRenderTargets(SIMULTANEOUS_RENDER_TARGETS, RenderTargetViews, DepthTargetView);

	//for (UINT i = 0; i < SIMULTANEOUS_RENDER_TARGETS; i++){
	//	if( RenderTargetViews[i] != NULL){
	//		DContext->ClearRenderTargetView(RenderTargetViews[i], ClearColor);
	//		SAFE_RELEASE(RenderTargetViews[i]);
	//	}
	//}

	//if(DepthTargetView){
	//	DContext->ClearDepthStencilView(DepthTargetView, D3D11_CLEAR_DEPTH, 0, 0);

	//}
	//SAFE_RELEASE(DepthTargetView);

	// clear back buffer
	DeviceContext->ClearRenderTargetView(RenderTargetView, ClearColor);
	
	// clear depth buffer
	DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


}
void Dx11Renderer::EndRender(){
	if(Vsync){
		// Lock to screen refresh rate
		SwapChain->Present(1, 0);
	} else {
		// present output //
		SwapChain->Present(0,0);
	}
}

//---------------------------------

void  Dx11Renderer::GetProjectionMatrix(D3DXMATRIX& projectionMatrix){
	projectionMatrix = ProjectionMatrix;
	return;
}
void  Dx11Renderer::GetWorldMatrix(D3DXMATRIX& worldMatrix){
	worldMatrix = WorldMatrix;
	return;
}
void  Dx11Renderer::GetOrthoMatrix(D3DXMATRIX& orthoMatrix){
	orthoMatrix = OrthoMatrix;
	return;
}

void  Dx11Renderer::GetVideoCardInfo(wstring& name, int& memory){
	name = VideoCardDesc;
	memory = VideoCardMemory;
}
// ------------------------------- //
void Dx11Renderer::TurnZBufferOn(){
	DeviceContext->OMSetDepthStencilState(DepthStencilState, 1);
}


void Dx11Renderer::TurnZBufferOff(){
	DeviceContext->OMSetDepthStencilState(DepthDisabledStencilState, 1);
}

void Dx11Renderer::TurnOnAlphaBlending()
{
	float blendFactor[4];
	
	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	
	// Turn on the alpha blending.
	DeviceContext->OMSetBlendState(AlphaEnableBlendingState, blendFactor, 0xffffffff);
}

void Dx11Renderer::TurnOffAlphaBlending()
{
	float blendFactor[4];
	
	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	
	// Turn off the alpha blending.
	DeviceContext->OMSetBlendState(AlphaDisableBlendingState, blendFactor, 0xffffffff);
}
