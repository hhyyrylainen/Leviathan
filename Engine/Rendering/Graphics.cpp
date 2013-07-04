#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GRAPHS
#include "Graphics.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "AppDefine.h"
#include "Application.h"
Graphics::Graphics(){
	Initialized = false;
	DxRenderer = false;
		
	Dconfig = DxRendConf();
	
	Drenderer = NULL;
	ActiveCamera = NULL;
	Shaders = NULL;
	Light = NULL;
	TextureKeeper = NULL;

	TextRender = NULL;

	GuiSmooth = 5;

	gadapter = this;
}
Graphics::~Graphics(){
	if(Initialized){
		Release();
	}
}

Graphics* Graphics::Get(){
	return gadapter;
}

Graphics* Graphics::gadapter = NULL;
// ------------------------------------------- //
bool Graphics::Init(Window* wind){
	// save window handle //
	Wind = wind;
	GuiSmooth = 5;
	// smoothness factor //
	AppDef::GetDefault()->GetValues()->GetValue(L"GuiSmooth").ConvertAndAssingToVariable<int>(GuiSmooth);

	if(!SUCCEEDED(Create3DRenderer(wind))){
		Logger::Get()->Error(L"Failed to init graphics, can't create 3drenderer");
		return false;
	}
	DxRenderer = true;
	
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

	CleanUpRenderActions();
	
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
	SAFE_RELEASEDEL(TextRender);

	Destroy3DRenderer();
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

	CurrentPass.release();

	// 2d rendering //
	ActiveCamera->GetStaticViewMatrix(ViewMatrix);
	Drenderer->GetWorldMatrix(WorldMatrix);
	Drenderer->GetProjectionMatrix(ProjectionMatrix);
	Drenderer->GetOrthoMatrix(OrthoMatrix);

	Drenderer->TurnZBufferOff();
	// turn on alpha blending //
	Drenderer->TurnOnAlphaBlending();

	//Bitmap->Render(Drenderer->GetDeviceContext(), this->Wind->GetWidth()-150, this->Wind->GetHeight()-150);
	//if(!Shaders->RenderMultiTextureShader(Drenderer->GetDeviceContext(), Bitmap->GetIndexCount(), WorldMatrix, ViewMatrix, OrthoMatrix, Bitmap->GetTextureArray())){

	//	return false;
	//}
	
	// render gui //
	//cquad->Render(Drenderer->GetDeviceContext(), 50, 50, this->Wind->GetWidth(), this->Wind->GetHeight(), 400, 250);

	//Shaders->RenderGradientShader(Drenderer->GetDeviceContext(), cquad->GetIndexCount(), WorldMatrix, ViewMatrix, OrthoMatrix, Float4(0.3f, 0.9f, 1.f, 1.f), Float4(0.3f, 0.4f, 1.f, 1.f));

	DrawRenderActions(WorldMatrix, ViewMatrix, OrthoMatrix);

	// text rendering // DON*T call this, gui render actions will call text rendering in right order
	//if(!TextRender->Render(Drenderer->GetDeviceContext(), WorldMatrix, OrthoMatrix)){

	//	return false;
	//}



	// turn z-buffer back on
	Drenderer->TurnOffAlphaBlending();
	Drenderer->TurnZBufferOn();


	// present result //
	Drenderer->EndRender();

	return true;
}

// ------------------------------------------- //
bool Graphics::HasRenderer(){
	return DxRenderer;
}
void Graphics::RecreateRenderer(Window* wind){
	if(Initialized){
		Destroy3DRenderer();
		Initialized = false;
	} else {
		Logger::Get()->Info(L"Renderer doesn't exist, recreating anyways");
	}
	if(!SUCCEEDED(Create3DRenderer(wind))){
		Logger::Get()->Error(L"Failed to recreate 3drenderer",0);
		return;
	}
	Initialized = true;
}

// ------------------------------------------- //

bool Graphics::Resize(int newwidth, int newheight){
	// resize renderer //
	Drenderer->Resize();

	// cameras //
	//ActiveCamera->
	//return true;

	// hope that everything uses updated values //
	return true;
}

HRESULT Graphics::Create3DRenderer(Window* wind){
	HRESULT hr= S_OK;

	//Dconfig = DxRendConf(tempwind, tempvsync, tempscreendepth, tempscreennear, tempdtype, tempmsaa);

	Drenderer = new Dx11Renderer;
	hr = Drenderer->Init(wind, this->Dconfig);
	if(SUCCEEDED(hr)){
		DxRenderer = true;
	}
	return hr;
}
void Graphics::Destroy3DRenderer(){
	if(DxRenderer){
		SAFE_RELEASE(Drenderer);
		DxRenderer = false;
	}
}
// ------------------------------------------- //
int Graphics::CountTextRenderLength(wstring &text, wstring &font, float heightmod, bool IsAbsolute, bool TranslateSize){
	return TextRender->CountSentenceLength(text, font, heightmod, IsAbsolute, TranslateSize);
}
int Graphics::GetTextRenderHeight(wstring &font, float heightmod, bool IsAbsolute, bool TranslateSize){
	return TextRender->GetFontHeight(font, heightmod, IsAbsolute, TranslateSize);
}
// ------------------------------------------- //
TextRenderer* Graphics::GetTextRenderer(){
	return TextRender;
}
Dx11Renderer* Graphics::GetRenderer(){
	return Drenderer;
}
TextureManager* Graphics::GetTextureManager(){
	return TextureKeeper;
}
ShaderManager* Graphics::GetShader(){
	return Shaders;
}
// ------------------------------------------- //
// ------------------------------------------- //
//void Graphics::SubmitAction(RenderAction* act){
//
//	GuiActions.push_back(act);
//
//}
void Graphics::SubmitRenderBridge(const shared_ptr<RenderBridge> &brdg){
	GuiObjs.push_back(brdg);
}
shared_ptr<RenderBridge> Graphics::GetBridgeForGui(int actionid){
	// so no dead objects exist //
	PurgeGuiArray();
	for(unsigned int i = 0; i < GuiObjs.size(); i++){
		if((*GuiObjs[i]).ID == actionid)
			return GuiObjs[i];
	}

	return NULL;
}
void Graphics::PurgeGuiArray(){
	for(unsigned int i = 0; i < GuiObjs.size(); i++){
		if((*GuiObjs[i]).WantsToClose){
			GuiObjs.erase(GuiObjs.begin()+i);
			i--;
			continue;
		}
	}
}

void Graphics::CleanUpRenderActions(){
	for(unsigned int i = 0; i < GuiObjs.size(); i++){
		GuiObjs[i].reset();
		GuiObjs.erase(GuiObjs.begin()+i);
		i--;
	}
}
//void Graphics::ProcessRenderActionInput(){
//	PurgeGuiArray();
//	for(int i = 0; i < GuiActions.size(); i++){
//		RenderAction* curact = GuiActions[i];
//		// look for existing handler //
//		int index = -1;
//		for(int a = 0; a < GuiObjs.size(); a++){
//			if((*GuiObjs[a]).ID == curact->FromID){
//				index = a;
//				break;
//			}
//		}
//		if(index < 0){
//			// didn't exist, needs to create new //
//			RenderBridge* nyabridge = new RenderBridge(curact->FromID, false);
//			GuiObjs.push_back(shared_ptr<RenderBridge>(nyabridge));
//
//			index = GuiObjs.size()-1;
//			// interpret instructions into it //
//
//			//continue; // let fall through to interpret instructions //
//		}
//		// already existed //
//
//		// get pointer to bridge for this loop //
//		RenderBridge* rbridge = GuiObjs[index].get();
//
//		// check does slot already exist //
//		int slotindex = -1;
//		for(int a = 0; a < rbridge->DrawActions.size(); a++){
//			if(rbridge->DrawActions[a]->SlotID == curact->StoreSlot){
//
//				slotindex = a;
//				break;
//			}
//		}
//		if(slotindex < 0){
//			// didn't exist, needs to create new //
//			RenderingGBlob* tempptr = NULL;
//			// switch here based on render type //
//			switch(curact->Type){
//			case RENDERACTION_SQUARE:
//				{
//					ColorQuadRendBlob* temppyptr = new ColorQuadRendBlob(
//				}
//			break;
//
//			}
//
//			rbridge->DrawActions.push_back(tempptr);
//			slotindex = rbridge->DrawActions.size()-1;
//		}
//
//
//	}
//	GuiActions.clear();
//}

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
			if((*GuiObjs[i]).ZVal != z)
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
					if((*GuiObjs[i]).DrawActions[a]->RelativeZ != locz)
						continue;
					if((*GuiObjs[i]).Hidden)
						continue;
					// real action to render //
					RenderingGBlob* tempptr = (*GuiObjs[i]).DrawActions[a];
					
					// compare, which class it is //
					/*string thistest = string(typeid(ColorQuadRendBlob).raw_name());*/
					if(tempptr->IsThisType(GUIRENDERING_BLOB_TYPE_CQUAD)){

						// draw the quad //
						ColorQuadRendBlob* renderptr = reinterpret_cast<ColorQuadRendBlob*>(tempptr);

						// get values //
						Int2 pos;
						Int2 size;
						Float4 col1;
						Float4 col2;
						int floatstyle;
						bool absolute;
						renderptr->Get(pos,col1,col2,size,floatstyle, absolute);

						// check does quad exist, if not create //
						if(renderptr->CQuad == NULL){
							renderptr->CQuad = new ColorQuad();

							// init //

							renderptr->CQuad->Init(Drenderer->GetDevice(), Wind->GetWidth(), Wind->GetHeight(), size[0], size[1], floatstyle);

							
						}
						// render //


						renderptr->CQuad->Render(Drenderer->GetDeviceContext(), pos[0], pos[1], Wind->GetWidth(), Wind->GetHeight(), size[0], size[1], absolute, floatstyle);

						// call shader to render this //
						Shaders->RenderGradientShader(Drenderer->GetDeviceContext(), renderptr->CQuad->GetIndexCount(), WorldMatrix, ViewMatrix, OrthoMatrix, col1, col2);
						continue;
					}
					if(tempptr->IsThisType(GUIRENDERING_BLOB_TYPE_TEXT)){

						// draw the quad //
						BasicTextRendBlob* renderptr = reinterpret_cast<BasicTextRendBlob*>(tempptr);

						// get values //
						Int2 pos;
						Float4 colour;
						float sizemod;
						bool absolute;
						wstring font;
						wstring text;
						int textid;
						// Int2 &xypos, Float4 &color, float &size, wstring &text, wstring &font, bool &absolute, int& textid

						renderptr->Get(pos, colour, sizemod, text, font, absolute, textid);

						// check does text exist, if not create //
						if(!renderptr->HasText){
							// create //
							TextRender->CreateSentence(textid, (int)(text.size()*1.5f), Drenderer->GetDevice());
							renderptr->HasText = true; // VERY important line
						}
						if(renderptr->ConsumeUpdate()){
							// update //
							TextRender->UpdateSentenceID(textid,absolute, font, text, pos[0],pos[1], colour, sizemod, Drenderer->GetDeviceContext());
						}

						// render //
						TextRender->RenderSingle(textid, Drenderer->GetDeviceContext(), WorldMatrix, OrthoMatrix);

						continue;
					}

					// invalid type //


				}
			}



		}
	}
}

Window* Leviathan::Graphics::GetWindow(){
	return Wind;
}
