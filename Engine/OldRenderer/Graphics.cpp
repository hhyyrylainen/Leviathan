#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GRAPHS
#include "Graphics.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "AppDefine.h"
#include "Application.h"
#include "ShaderManager.h"
#include "RenderingQuad.h"

Graphics::Graphics() : ActiveCamera(NULL), Shaders(NULL), Light(NULL), TextureKeeper(NULL), TextRender(NULL), Initialized(false)
{

	GuiSmooth = 5;

	Staticaccess = this;
}
Graphics::~Graphics(){
}

Graphics* Graphics::Get(){
	return Staticaccess;
}

Graphics* Graphics::Staticaccess = NULL;
// ------------------------------------------- //
bool Graphics::Init(Window* wind){
	// save window handle //
	Wind = wind;

	// set resource creator to use this graphics //
	Rendering::ResourceCreator::StoreGraphicsInstance(this);

	// smoothness factor //
	ObjectFileProcessor::LoadValueFromNamedVars<int>(AppDef::GetDefault()->GetValues(), L"GuiSmooth", GuiSmooth, 5, false);


	
	// create texture holder //
	TextureKeeper = new TextureManager(true, this);
	if(!TextureKeeper){
		Logger::Get()->Error(L"Graphics: Init: 008");
		return false;
	}
	if(!TextureKeeper->Init(FileSystem::GetTextureFolder(), TEXTURE_INACTIVE_TIME, TEXTURE_UNLOAD_TIME)){

		Logger::Get()->Error(L"Graphics: Init: TextureKeeper failed to init");
		return false;
	}

	// create shader holder //
	Shaders = new Rendering::ShaderManager();
	if(!Shaders){
		Logger::Get()->Error(L"Failed to init graphics, can't create ShaderManager");
		return false;
	}

	// init it //
	if(!Shaders->Init()){

		Logger::Get()->Error(L"Failed to init graphics, can't init shaders");
		return false;
	}

	// create camera //
	ActiveCamera = new ViewCamera;
	if(!ActiveCamera){
		Logger::Get()->Error(L"Failed to init graphics, can't create Camera",GetLastError());
		return false;
	}

	// set camera pos //
	ActiveCamera->SetPosition(Float3(0.0f, 0.0f, -10.0f));
	// render to set matrices //
	ActiveCamera->UpdateMatrix();

	// Create the light object.
	Light = new RenderingLight;
	if(!Light){

		return false;
	}

	// Initialize the light object.
	Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	Light->SetDirection(0.5f, 0.0f, 1.0f);
	Light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	Light->SetSpecularPower(32.0f);

	// text renderer //
	TextRender = new TextRenderer;

	if(!TextRender){
		Logger::Get()->Error(L"Failed to init graphics, can't create TextRenderer");
		return false;
	}

	if(!TextRender->Init(this)){
		Logger::Get()->Error(L"Failed to init graphics, can't init TextRenderer");
		return false;
	}

	Initialized = true;
	return true;
}

void Graphics::Release(){

	GuiObjs.clear();

	SAFE_RELEASEDEL(TextRender);
	SAFE_DELETE(Light);
	SAFE_DELETE(ActiveCamera);

	SAFE_RELEASEDEL(TextureKeeper);
	SAFE_RELEASEDEL(Shaders);


	Initialized = false;
}
// ------------------------------------------- //
bool Graphics::Frame(int mspassed, ViewerCameraPos* camerapostouse, vector<BaseRenderable*> &objects){
	// set camera pos //
	float x, y, z, yaw, pitch, roll;
	camerapostouse->UpdatePos(mspassed);
	camerapostouse->GetPos(x,y,z);
	camerapostouse->GetRotation(yaw,pitch,roll);
	ActiveCamera->SetPosition(Float3(y,z,x));
	ActiveCamera->SetRotation(Float3(pitch, yaw, roll));

	// call rendering function //
	if(!Render(mspassed, objects)){

		return false;
	}

	return true;
}

bool Graphics::Render(int mspassed, vector<BaseRenderable*> &objects){
	// clear render target //
	Drenderer->BeginRender(Float4(0.4f, 0.7f, 0.85f, 1));

	// update camera //
	ActiveCamera->UpdateMatrix();
	
	// get matrices //
	D3DXMATRIX ViewMatrix, ProjectionMatrix, WorldMatrix, OrthoMatrix;

	ActiveCamera->GetViewMatrix(ViewMatrix);
	Drenderer->GetWorldMatrix(WorldMatrix);
	Drenderer->GetProjectionMatrix(ProjectionMatrix);

	// create info object about current pass //
	unique_ptr<RenderingPassInfo> CurrentPass = unique_ptr<RenderingPassInfo>(new RenderingPassInfo(ViewMatrix, ProjectionMatrix, WorldMatrix, 
		ActiveCamera));

	// go through objects and call their render functions //
	for(size_t i = 0; i < objects.size(); i++){
		if(objects[i]->IsHidden())
			continue;
		
		// objects should be smart enough to not to change common matrices
		objects[i]->Render(this, mspassed, *CurrentPass.get()); 
	}

	// 2d rendering //
	ActiveCamera->GetStaticViewMatrix(ViewMatrix);
	Drenderer->GetWorldMatrix(WorldMatrix);
	Drenderer->GetOrthoMatrix(OrthoMatrix);

	Drenderer->TurnZBufferOff();
	// turn on alpha blending //
	Drenderer->TurnOnAlphaBlending();

	// update pass object //
	CurrentPass->SetWorldMatrix(WorldMatrix);
	CurrentPass->SetViewMatrix(ViewMatrix);
	CurrentPass->SetProjectionMatrix(OrthoMatrix);

	DrawRenderActions(CurrentPass.get());

	// turn z-buffer back on
	Drenderer->TurnOffAlphaBlending();
	Drenderer->TurnZBufferOn();



	// present result //
	Drenderer->EndRender();
	return true;
}
// ------------------------------------------- //
bool Graphics::Resize(int newwidth, int newheight){
	// resize renderer //
	Drenderer->Resize();

	// hope that everything uses updated values //
	return true;
}
// ------------------------------------------- //
void Graphics::SubmitRenderBridge(const shared_ptr<RenderBridge> &brdg){
	GuiObjs.push_back(brdg);
}
shared_ptr<RenderBridge> Graphics::GetBridgeForGui(int actionid){
	// so no dead objects exist //
	PurgeGuiArray();
	for(unsigned int i = 0; i < GuiObjs.size(); i++){
		if(GuiObjs[i]->ID == actionid)
			return GuiObjs[i];
	}

	return NULL;
}
void Graphics::PurgeGuiArray(){
	for(unsigned int i = 0; i < GuiObjs.size(); i++){
		if(GuiObjs[i]->WantsToClose){
			GuiObjs.erase(GuiObjs.begin()+i);
			i--;
			continue;
		}
	}
}
// ------------------------------------------- //
void Graphics::DrawRenderActions(RenderingPassInfo* pass){
	// so no dead objects exist //
	PurgeGuiArray();

	int MinZ = 0;
	int MaxZ = 1;

	for(unsigned int i = 0; i < GuiObjs.size(); i++){
		int curz = (*GuiObjs[i]).ZVal;
		if(curz < MinZ)
			MinZ = curz;
		if(curz > MaxZ)
			MaxZ = curz;
	}
	// draw everything in order //
	for(int z = MinZ; z <= MaxZ; z++){
		// loop through objects //
		for(unsigned int i = 0; i < GuiObjs.size(); i++){
			if(GuiObjs[i]->Hidden || GuiObjs[i]->ZVal != z)
				continue;
			// render //
			int locmax = 10;
			for(unsigned int a = 0; a < (*GuiObjs[i]).DrawActions.size(); a++){
				if((*GuiObjs[i]).DrawActions[a]->RelativeZ > locmax)
					locmax = (*GuiObjs[i]).DrawActions[a]->RelativeZ;
			}

			// draw this object's children //
			for(int locz = 0; locz <= locmax; locz++){
				for(unsigned int a = 0; a < (*GuiObjs[i]).DrawActions.size(); a++){
					if(GuiObjs[i]->DrawActions[a]->RelativeZ != locz || GuiObjs[i]->Hidden)
						continue;

					// real action to render //
					RenderingGBlob* tempptr = (*GuiObjs[i]).DrawActions[a];

					// with virtual objects it is as easy as this //
					if(!RenderAutomatic(tempptr->GetRenderingBuffers(this), tempptr->GetShaderParameters(this, pass))){


						continue;
					}
				}
			}
		}
	}
}

DLLEXPORT bool Leviathan::Graphics::RenderAutomatic(Rendering::BaseRenderableBufferContainer* torenderbuffers, ShaderRenderTask* shaderparameters){
	// finalize render task //
	shaderparameters->SetInputPattern(torenderbuffers->GetInputFormat());

#ifdef _DEBUG
	string tmpformatstrcheck = shaderparameters->GetShaderPattern();
#endif // _DEBUG


	// find matching shader //
	BaseShader* tmpsptr = Shaders->GetShaderMatchingObject(shaderparameters, torenderbuffers->GetPreferredShaderName());

	if(!tmpsptr){
		// no matching shader found //
		return false;
	}

	// final check on object //
	if(!tmpsptr->DoesInputObjectWork(shaderparameters)){

		return false;
	}

	// set buffers to directx //
	int DrawIndexCount = 0;
	if(!torenderbuffers->SetBuffersForRendering(Drenderer->GetDeviceContext(), DrawIndexCount)){
		// buffers cannot be rendered, abort rendering //
		return false;
	}

	// call shader render //
	if(!tmpsptr->Render(Drenderer->GetDeviceContext(), DrawIndexCount, shaderparameters)){
		// shader failed to update internal buffers //
		return false;
	}


	// even if the rendering at this point could fail we count getting this far as success //
	return true;
}
