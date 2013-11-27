#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_MAIN
#include "GuiManager.h"
#endif
#include "Engine.h"
#include "Script\ScriptInterface.h"
#include "FileSystem.h"
#include "Rendering\Graphics.h"
#include <boost\assign\list_of.hpp>
#include <Rocket\Controls\Controls.h>
#include <Rocket\Debugger\Debugger.h>
#include <Rocket/Controls.h>
#include <Rocket/Debugger.h>
#include "Rendering\GUI\RenderInterfaceOgre3D.h"
#include "Rendering/GUI/FontManager.h"
#include "GuiCollection.h"
#include "Common\DataStoring\DataStore.h"
#include "Common\DataStoring\DataBlock.h"
#include "Common\GraphicalInputEntity.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
Leviathan::Gui::GuiManager::GuiManager() : ID(IDFactory::GetID()), RocketRenderer(NULL), RocketInternals(NULL), WindowContext(NULL), Visible(true),
	Cursor(NULL), GuiMouseUseUpdated(true), GuiDisallowMouseCapture(true)
{

	staticaccess = this;
}
Leviathan::Gui::GuiManager::~GuiManager(){

	staticaccess = NULL;
}

GuiManager* Leviathan::Gui::GuiManager::staticaccess = NULL;
GuiManager* Leviathan::Gui::GuiManager::Get(){
	return staticaccess; 
}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::Init(AppDef* vars, Graphics* graph, GraphicalInputEntity* window){

	ThisWindow = window;

	Window* wind = window->GetWindow();

	// create renderer for Rocket GUI //
	RocketRenderer = new RenderInterfaceOgre3D(wind->GetWidth(), wind->GetHeight());

	Rocket::Core::SetRenderInterface(RocketRenderer);

	RocketInternals = new RocketSysInternals();

	Rocket::Core::SetSystemInterface(RocketInternals);

	Rocket::Core::Initialise();
	Rocket::Controls::Initialise();

	// font database //
	Rocket::Core::FontDatabase::Initialise();

	// load fonts //
	graph->GetFontManager()->LoadAllFonts();

	// create context for this window //
	WindowContext = Rocket::Core::CreateContext("window01_context", Rocket::Core::Vector2i(wind->GetWidth(), wind->GetHeight()));

	// initialize debugger only once //
	if(!RocketDebuggerInitialized){
		Rocket::Debugger::Initialise(WindowContext);
		RocketDebuggerInitialized = true;
	}

	// we render during Ogre overlay //
	wind->GetOverlayScene()->addRenderQueueListener(this);

	// set to be rendered in right viewport //
	Graphics::Get()->GetOverlayMaster()->SetGUIVisibleInViewport(this, ThisWindow->GetWindow()->GetOverlayViewport());

	return true;
}

void Leviathan::Gui::GuiManager::Release(){
	// default mouse back //
	SetMouseFile(L"none");

	for(unsigned int i = 0; i < Objects.size(); i++){
		// object's release function will do everything needed (even deleted if last reference) //
		SAFE_RELEASE(Objects[i]);
	}
	Objects.clear();

	// GuiCollections are now also reference counted //
	for(size_t i = 0; i < Collections.size(); i++){
		SAFE_RELEASE(Collections[i]);
	}
	Collections.clear();

	GuiSheets.clear();


	// shutdown rocket //
	WindowContext->RemoveReference();
	Rocket::Core::Shutdown();
	Rocket::Core::FontDatabase::Shutdown();

	SAFE_DELETE(RocketInternals);
	SAFE_DELETE(RocketRenderer);

	// we can clear this too //
	BaseGuiObject::LeviathanToRocketEventTranslate.clear();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::ProcessKeyDown(OIS::KeyCode key, int specialmodifiers){

	for(unsigned int i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetTogglingKey().Match(key, specialmodifiers, false) && Collections[i]->GetAllowEnable()){
			// is a match, toggle //
			Collections[i]->ToggleState();

			return true;
		}
	}

	return false;
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionStateProxy(string name, bool state){
	// call the actual function //
	SetCollectionState(Convert::StringToWstring(name), state);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionState(const wstring &name, bool state){
	// find collection with name and set it's state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetState() != state){
				Logger::Get()->Info(L"Setting Collection "+Collections[i]->GetName()+L" state "+Convert::ToWstring(state));
				Collections[i]->ToggleState();
			}
			return;
		}
	}
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionAllowEnableState(const wstring &name, bool allow /*= true*/){
	// find collection with name and set it's allow enable state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetAllowEnable() != allow){
				Logger::Get()->Info(L"Setting Collection "+Collections[i]->GetName()+L" allow enable state "+Convert::ToWstring(allow));
				Collections[i]->ToggleAllowEnable();
			}
			return;
		}
	}
}
// ------------------------------------ //
void Leviathan::Gui::GuiManager::GuiTick(int mspassed){
	// send tick event //



	// check if we want mouse //
	if(GuiMouseUseUpdated){
		GuiMouseUseUpdated = false;

		// scan if any collections keep GUI active //
		bool active = false;


		for(size_t i = 0; i < Collections.size(); i++){

			if(Collections[i]->KeepsGUIActive()){
				active = true;
				break;
			}
		}

		if(active != GuiDisallowMouseCapture){
			// state updated //
			GuiDisallowMouseCapture = active;

			if(GuiDisallowMouseCapture){
				// disable mouse capture //
				ThisWindow->SetMouseCapture(false);

			} else {

				// activate direct mouse capture //
				if(!ThisWindow->SetMouseCapture(true)){
					// failed, GUI must be forced to stay on //
					OnForceGUIOn();
					GuiDisallowMouseCapture = true;
					GuiMouseUseUpdated = true;
				}
			}
		}
	}
}

DLLEXPORT void Leviathan::Gui::GuiManager::OnForceGUIOn(){
	DEBUG_BREAK;
}

void Leviathan::Gui::GuiManager::Render(){

	// update Rocket input //
	ThisWindow->GetWindow()->GatherInput(WindowContext);
}
// ------------------------------------ //
void Leviathan::Gui::GuiManager::OnResize(int width, int height){
	// call events //
	this->CallEvent(new Event(EVENT_TYPE_WINDOW_RESIZE, (void*)new Int2(width, height)));

	// resize Rocket on this window //
	WindowContext->SetDimensions(Rocket::Core::Vector2i(width, height));
}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::AddGuiObject(BaseGuiObject* obj){
	Objects.push_back(obj);
	return true;
}

void Leviathan::Gui::GuiManager::DeleteObject(int id){
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id){

			SAFE_RELEASE(Objects[i]);
			Objects.erase(Objects.begin()+i);
			return;
		}
	}
}

int Leviathan::Gui::GuiManager::GetObjectIndexFromId(int id){
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id)
			return i;
	}
	return -1;
}

BaseGuiObject* Leviathan::Gui::GuiManager::GetObject(unsigned int index){
	ARR_INDEX_CHECK(index, Objects.size()){
		return Objects[index];
	}
	return NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::LoadGUIFile(const wstring &file){
	// header flag definitions //
	vector<shared_ptr<NamedVariableList>> headerdata;

	// parse file to structure //
	vector<shared_ptr<ObjectFileObject>> data = ObjectFileProcessor::ProcessObjectFile(file, headerdata);

	NamedVars varlist(headerdata);

	// we need to load the corresponding rocket file first //
	wstring relativepath;
	// get path //
	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(varlist, L"RocketScript", relativepath, L"", true,
		L"GuiManager: LoadGUIFile: No Rocket script file attached: ");

	if(!relativepath.size()){

		return false;
	}
	
	shared_ptr<GuiLoadedSheet> sheet;
	
	try{
		sheet = shared_ptr<GuiLoadedSheet>(new GuiLoadedSheet(WindowContext, Convert::WstringToString(FileSystem::GetScriptsFolder()+relativepath)),
		//	std::mem_fun_ref(&GuiLoadedSheet::ReleaseProxy));
			[](GuiLoadedSheet* p){p->Release();});
		if(!sheet.get()){

			return false;
		}
	}
	catch(const ExceptionInvalidArguement &e){
		// something was thrown //
		Logger::Get()->Error(L"GuiManager: LoadGUIFile: exit due to exception:");
		e.PrintToLog();

		return false;
	}

	// temporary object data stores //
	vector<BaseGuiObject*> TempOs;

	// reserve space //
	TempOs.reserve(data.size());


	for(size_t i = 0; i < data.size(); i++){
		// check what type the object is //
		if(data[i]->TName == L"GuiCollection" || data[i]->TName == L"Collection"){

			if(!GuiCollection::LoadCollection(this, *data[i], sheet.get())){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load collection, named "+data[i]->Name);
				continue;
			}
			// delete rest of the object //
			goto guiprocessguifileloopdeleteprocessedobject;
		}
		if(data[i]->TName == L"GuiObject"){

			// try to load //
			if(!BaseGuiObject::LoadFromFileStructure(this, TempOs, *data[i], sheet.get())){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load GuiObject, named "+data[i]->Name);
				continue;
			}
			// delete rest of the object //
			goto guiprocessguifileloopdeleteprocessedobject;
		}

		Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: Unrecognized type! typename: "+data[i]->TName);

guiprocessguifileloopdeleteprocessedobject:

		// delete current //
		data.erase(data.begin()+i);
		i--;
	}

	GuiSheets.insert(make_pair(sheet->GetID(), sheet));

	for(size_t i = 0; i < TempOs.size(); i++){

		// add to real objects //
		AddGuiObject(TempOs[i]);
	}

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::SetMouseFile(const wstring &file){

	if(file == L"none" || Cursor){

		if(Cursor){
			Cursor->Close();
			Cursor->RemoveReference();
			Cursor = NULL;
		}
		// show default window cursor //
		ThisWindow->GetWindow()->SetHideCursor(false);
		if(file == L"none")
			return;
	}


	Cursor = WindowContext->LoadMouseCursor(Convert::WstringToString(file).c_str());

	if(Cursor){

		Cursor->Show();
		// hide window cursor //
		ThisWindow->GetWindow()->SetHideCursor(true);
	}
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetMouseFileVisibleState(bool state){

	WindowContext->ShowMouseCursor(state);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetDebuggerOnThisContext(){

	Rocket::Debugger::SetContext(WindowContext);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetDebuggerVisibility(bool visible){
	Rocket::Debugger::SetVisible(visible);
}

// ----------------- event handler part --------------------- //
bool Leviathan::Gui::GuiManager::CallEvent(Event* pEvent){
	// loop through listeners and call events //
	for(size_t i = 0; i < Objects.size(); i++){
		// call
		Objects[i]->OnEvent(&pEvent);
		// check for deletion and end if event got deleted //
		if(!pEvent){
			return true;
		}
	}

	// delete object ourselves //
	SAFE_DELETE(pEvent);
	// nobody wanted the event //
	return false;
}
// used to send hide events to individual objects //
int GuiManager::CallEventOnObject(BaseGuiObject* receive, Event* pEvent){
	// find right object
	int returval = -3;

	for(size_t i = 0; i < Objects.size(); i++){
		if(Objects[i] == receive){
			// call
			returval = Objects[i]->OnEvent(&pEvent);
			break;
		}
	}

	// delete object ourselves (if it still exists) //
	SAFE_DELETE(pEvent);
	return returval;
}
// ----------------- collection managing --------------------- //
void GuiManager::AddCollection(GuiCollection* add){
	Collections.push_back(add);
}
GuiCollection* Leviathan::Gui::GuiManager::GetCollection(const int &id, const wstring &name){
	// look for collection based on id or name //
	for(size_t i = 0; i < Collections.size(); i++){
		if(id >= 0){
			if(Collections[i]->GetID() != id){
				// no match //
				continue;
			}
		} else {
			// name should be specified, check for it //
			if(Collections[i]->GetName() != name){
				continue; 
			}
		}

		// match
		return Collections[i];
	}

	return NULL;
}
// -------------------------------------- //
// code from Rocket Ogre sample //
void Leviathan::Gui::GuiManager::renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation){
	// we render Rocket at the same time with OGRE overlay //
	if(queueGroupId == Ogre::RENDER_QUEUE_OVERLAY && Visible){

		WindowContext->Update();

		ConfigureRenderSystem();
		WindowContext->Render();
	}
}

// Configures Ogre's rendering system for rendering Rocket.
void Leviathan::Gui::GuiManager::ConfigureRenderSystem()
{
	Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();

	// Set up the projection and view matrices.
	Ogre::Matrix4 projection_matrix;
	BuildProjectionMatrix(projection_matrix);
	render_system->_setProjectionMatrix(projection_matrix);
	render_system->_setViewMatrix(Ogre::Matrix4::IDENTITY);

	// Disable lighting, as all of Rocket's geometry is unlit.
	render_system->setLightingEnabled(false);
	// Disable depth-buffering; all of the geometry is already depth-sorted.
	render_system->_setDepthBufferParams(false, false);
	// Rocket generates anti-clockwise geometry, so enable clockwise-culling.
	render_system->_setCullingMode(Ogre::CULL_CLOCKWISE);
	// Disable fogging.
	render_system->_setFog(Ogre::FOG_NONE);
	// Enable writing to all four channels.
	render_system->_setColourBufferWriteEnabled(true, true, true, true);
	// Unbind any vertex or fragment programs bound previously by the application.
	render_system->unbindGpuProgram(Ogre::GPT_FRAGMENT_PROGRAM);
	render_system->unbindGpuProgram(Ogre::GPT_VERTEX_PROGRAM);

	// Set texture settings to clamp along both axes.
	Ogre::TextureUnitState::UVWAddressingMode addressing_mode;
	addressing_mode.u = Ogre::TextureUnitState::TAM_CLAMP;
	addressing_mode.v = Ogre::TextureUnitState::TAM_CLAMP;
	addressing_mode.w = Ogre::TextureUnitState::TAM_CLAMP;
	render_system->_setTextureAddressingMode(0, addressing_mode);

	// Set the texture coordinates for unit 0 to be read from unit 0.
	render_system->_setTextureCoordSet(0, 0);
	// Disable texture coordinate calculation.
	render_system->_setTextureCoordCalculation(0, Ogre::TEXCALC_NONE);
	// Enable linear filtering; images should be rendering 1 texel == 1 pixel, so point filtering could be used
	// except in the case of scaling tiled decorators.
	render_system->_setTextureUnitFiltering(0, Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_POINT);
	// Disable texture coordinate transforms.
	render_system->_setTextureMatrix(0, Ogre::Matrix4::IDENTITY);
	// Reject pixels with an alpha of 0.
	render_system->_setAlphaRejectSettings(Ogre::CMPF_GREATER, 0, false);
	// Disable all texture units but the first.
	render_system->_disableTextureUnitsFrom(1);

	// Enable simple alpha blending.
	render_system->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

	// Disable depth bias.
	render_system->_setDepthBias(0, 0);
}

// Builds an OpenGL-style orthographic projection matrix.
void Leviathan::Gui::GuiManager::BuildProjectionMatrix(Ogre::Matrix4& projection_matrix)
{
	float z_near = -1;
	float z_far = 1;

	projection_matrix = Ogre::Matrix4::ZERO;

	// Set up matrices.
	projection_matrix[0][0] = 2.0f / ThisWindow->GetWindow()->GetWidth();
	projection_matrix[0][3]= -1.0000000f;
	projection_matrix[1][1]= -2.0f / ThisWindow->GetWindow()->GetHeight();
	projection_matrix[1][3]= 1.0000000f;
	projection_matrix[2][2]= -2.0f / (z_far - z_near);
	projection_matrix[3][3]= 1.0000000f;
}

DLLEXPORT void Leviathan::Gui::GuiManager::GUIObjectsCheckRocketLinkage(){
	for(size_t i = 0; i < Objects.size(); i++){

		Objects[i]->CheckObjectLinkage();
	}
}





bool Leviathan::Gui::GuiManager::RocketDebuggerInitialized = false;


