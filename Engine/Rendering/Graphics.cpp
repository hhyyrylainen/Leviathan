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

Graphics::Graphics(){
	Initialized = false;
		
	Dconfig = DxRendConf();
	
	Drenderer = NULL;
	ActiveCamera = NULL;
	Shaders = NULL;
	Light = NULL;
	TextureKeeper = NULL;

	TextRender = NULL;

	GuiSmooth = 5;

	_gadapter = this;
}
Graphics::~Graphics(){
	if(Initialized){
		Release();
	}
}

Graphics* Graphics::Get(){
	return _gadapter;
}

Graphics* Graphics::_gadapter = NULL;
// ------------------------------------------- //
bool Graphics::Init(Window* wind, const DxRendConf &conf){
	// save window handle //
	Wind = wind;
	GuiSmooth = 5;
	// set description object //
	SetDescObjects(conf);

	// set resource creator to use this graphics //
	Rendering::ResourceCreator::StoreGraphicsInstance(this);

	// smoothness factor //
	ObjectFileProcessor::LoadValueFromNamedVars<int>(AppDef::GetDefault()->GetValues(), L"GuiSmooth", GuiSmooth, 5, false);

	Drenderer = new Dx11Renderer();
	if(!SUCCEEDED(Drenderer->Init(wind, this->Dconfig))){
		Logger::Get()->Error(L"Failed to init graphics, can't create 3drenderer");
		return false;
	}
	
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
	Shaders = new ShaderManager;
	if(!Shaders){
		Logger::Get()->Error(L"Failed to init graphics, can't create ShaderManager");
		return false;
	}

	// init it //
	if(!Shaders->Init(Drenderer->GetDevice())){

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
	D3DXMATRIX tempview;
	ActiveCamera->GetStaticViewMatrix(tempview);

	if(!TextRender->Init(Drenderer->GetDevice(), Drenderer->GetDeviceContext(), wind, tempview)){
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
	try{
		SAFE_RELEASEDEL(TextureKeeper);
	}
	// this is tried because it might cause an assertion, which is something we don't like //
	catch(...){
		Logger::Get()->Error(L"Possible assertion from releasing graphics");
	}
	SAFE_RELEASEDEL(Shaders);


	SAFE_RELEASEDEL(Drenderer);
	Initialized = false;
}
// ------------------------------------------- //
bool Graphics::Frame(int mspassed, ViewerCameraPos* camerapostouse, vector<BaseRenderable*> &objects){
	if(Initialized){


		//ProcessRenderActionInput();
		//float rotation = 0.0f;

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
	}

	return true;
}
bool Graphics::Render(int mspassed, vector<BaseRenderable*> &objects){

	// clear render target //
	Drenderer->BeginRender(Float4(0.4f, 0.7f, 0.85f, 1));

	// update camera //
	ActiveCamera->UpdateMatrix();
	
	// get matrices //
	D3DXMATRIX ViewMatrix, ProjectionMatrix, WorldMatrix, TranslateMatrix, OrthoMatrix;

	ActiveCamera->GetViewMatrix(ViewMatrix);
	Drenderer->GetWorldMatrix(WorldMatrix);
	Drenderer->GetProjectionMatrix(ProjectionMatrix);

	// create info object about current pass //
	unique_ptr<RenderingPassInfo> CurrentPass = unique_ptr<RenderingPassInfo>(new RenderingPassInfo());

	// go through objects and call their render functions //
	for(unsigned int i = 0; i < objects.size(); i++){
		if(objects[i]->IsHidden())
			continue;
		// redo matrices and call render //
		//Drenderer->GetWorldMatrix(WorldMatrix); // objects should be smart enough to not to change common matrices

		objects[i]->Render(this, mspassed, *CurrentPass.get(), ViewMatrix, ProjectionMatrix, WorldMatrix, TranslateMatrix, ActiveCamera->GetPosition()); 

		// reset pass object //
		CurrentPass->ResetState();
	}

	// 2d rendering //
	ActiveCamera->GetStaticViewMatrix(ViewMatrix);
	Drenderer->GetWorldMatrix(WorldMatrix);
	Drenderer->GetProjectionMatrix(ProjectionMatrix);
	Drenderer->GetOrthoMatrix(OrthoMatrix);

	Drenderer->TurnZBufferOff();
	// turn on alpha blending //
	Drenderer->TurnOnAlphaBlending();

	DrawRenderActions(WorldMatrix, ViewMatrix, OrthoMatrix);

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


void Graphics::DrawRenderActions(D3DXMATRIX WorldMatrix, D3DXMATRIX ViewMatrix, D3DXMATRIX OrthoMatrix){
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
					if(!RenderAutomatic(tempptr->GetRenderingBuffers(this), tempptr->GetShaderParameters(this))){


						continue;
					}

					if(tempptr->IsThisType(GUIRENDERING_BLOB_TYPE_TEXT)){

						// draw the quad //
						BasicTextRendBlob* renderptr = reinterpret_cast<BasicTextRendBlob*>(tempptr);

						// get values //
						Float2 pos;
						Float4 colour;
						float sizemod;
						int coordtype;
						wstring font;
						wstring text;
						int textid;
						// get data from render blob //
						renderptr->Get(pos, colour, sizemod, text, font, coordtype, textid);

						// check does text exist, if not create //
						if(!renderptr->HasText){
							// create //
							TextRender->CreateSentence(textid, (int)(text.size()*1.5f), Drenderer->GetDevice());
							renderptr->HasText = true; // VERY important line
						}
						if(renderptr->ConsumeUpdate()){
							// update //
							TextRender->UpdateSentenceID(textid, coordtype, font, text, pos, colour, sizemod, Drenderer->GetDeviceContext());
						}

						// render //
						TextRender->RenderSingle(textid, Drenderer->GetDeviceContext(), WorldMatrix, OrthoMatrix);

						continue;
					}
					if(tempptr->IsThisType(GUIRENDERING_BLOB_TYPE_EXPENSIVETEXT)){

						// cast pointer to right type //
						ExpensiveTextRendBlob* renderptr = reinterpret_cast<ExpensiveTextRendBlob*>(tempptr);


						// call drawing with object //
						TextRender->RenderExpensiveText(renderptr, Drenderer->GetDeviceContext(), WorldMatrix, OrthoMatrix);


						continue;
					}

					// invalid type //


				}
			}



		}
	}
}
