#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GRAPHICS
#include "Graphics.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Application\AppDefine.h"
#include "Application\Application.h"
#include "Utility\ComplainOnce.h"
#include <boost\assign\list_of.hpp>

Graphics::Graphics() : Light(NULL), TextureKeeper(NULL), ORoot(nullptr), MainCamera(NULL), MainCameraNode(NULL), MainViewport(NULL)
{
	GuiSmooth = 5;

	Staticaccess = this;
	Initialized = false;
}
Graphics::~Graphics(){
}

Graphics* Graphics::Get(){
	return Staticaccess;
}

Graphics* Graphics::Staticaccess = NULL;
// ------------------------------------------- //
bool Graphics::Init(AppDef* appdef){
	// save definition pointer //
	AppDefinition = appdef;

	// smoothness factor //
	ObjectFileProcessor::LoadValueFromNamedVars<int>(AppDef::GetDefault()->GetValues(), L"GuiSmooth", GuiSmooth, 5, false);

	// create ogre renderer //
	if(!InitializeOgre(AppDefinition)){

		Logger::Get()->Error(L"Graphics: Init: failed to create ogre renderer");
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

	Initialized = true;
	return true;
}

void Graphics::Release(){


	GuiObjs.clear();
	SAFE_DELETE(Light);
	SAFE_RELEASEDEL(TextureKeeper);


	Initialized = false;
}
// ------------------------------------------- //
bool Leviathan::Graphics::InitializeOgre(AppDef* appdef){

	Ogre::String ConfigFileName = "";


	Ogre::String PluginsFileName = "";

	Ogre::String OgreLogName = "LogOGRE.txt";

	ORoot = unique_ptr<Ogre::Root>(new Ogre::Root(PluginsFileName, ConfigFileName, OgreLogName));


	vector<Ogre::String> PluginNames = boost::assign::list_of("RenderSystem_GL")("Plugin_ParticleFX")("Plugin_CgProgramManager")
		("Plugin_OctreeSceneManager");

	for(auto Iter = PluginNames.begin(); Iter != PluginNames.end(); Iter++){
		// append "_d" if in debug mode //
#ifdef _DEBUG
		Iter->append("_d");
#endif // _DEBUG
		// load //
		ORoot->loadPlugin(*Iter);
	}


	// Choose proper render system //
	const Ogre::RenderSystemList& RSystemList = ORoot->getAvailableRenderers();

	if(RSystemList.size() == 0){
		// no render systems found //

		Logger::Get()->Error(L"Graphics: InitializeOgre: no render systems found");
		return false;
	}

	// for now just choose the first one in the list //
	ORoot->setRenderSystem(RSystemList[0]);


	bool CreateWindowNow = false;
	Ogre::String WindowTitle = "";
	Ogre::String CustomCapacities = "";

	ORoot->initialise(CreateWindowNow, WindowTitle, CustomCapacities);


	// we can now ourselves create a window //
	const WindowDataDetails& WData = AppDefinition->GetWindowDetails();

	// set some rendering specific parameters //

	Ogre::NameValuePairList WParams;

	WParams["FSAA"] = "0";
	WParams["vsync"] = "false";

	Ogre::String wcaption = Convert::WstringToString(WData.Title);

	// create the actual window and store it at the same time //
	AppDefinition->SetRenderingWindow(ORoot->createRenderWindow(wcaption, WData.Width, WData.Height, !WData.Windowed, &WParams));

	if(!CreateDefaultRenderView()){

		return false;
	}

	// quicker access to the window //
	Ogre::RenderWindow* tmpwindow = AppDefinition->GetWindow();

	// set the main window to be active //
	tmpwindow->setActive(true);

	// manual window updating //
	tmpwindow->setAutoUpdated(false);


	// set loading paths //
	FileSystem::RegisterOGREResourceGroups();

	CreateTestObject();

	// create test model //
	try{
		Ogre::Entity* ModelEntity = MainScene->createEntity("Cube.mesh");

		// attach to new node //
		Ogre::SceneNode* mnode = MainScene->getRootSceneNode()->createChildSceneNode();

		mnode->attachObject(ModelEntity);
		// set position //
		mnode->setPosition(0.f, -2.f, -5.f);
	}
	catch(const Ogre::FileNotFoundException &e){

		Logger::Get()->Error(L"[EXCEPTION] "+Convert::StringToWstring(e.getFullDescription()));
	}
	
	
	// test light //
	Ogre::Light* TLight = MainScene->createLight();

	TLight->setType(Ogre::Light::LT_DIRECTIONAL);

	TLight->setDiffuseColour(0.98f, 1.f, 0.95f);
	TLight->setSpecularColour(1.f, 1.f, 1.f);

	Ogre::SceneNode* lnode = MainScene->getRootSceneNode()->createChildSceneNode();
	lnode->attachObject(TLight);
	lnode->setOrientation(1.f, 0.5f, -0.3f, 0.4f);

	// set scene ambient colour //
	MainScene->setAmbientLight(Ogre::ColourValue(0.2f, 0.2f, 0.2f, 1.f));




	// clear events that might have queued A LOT while starting up //
	ORoot->clearEventTimes();

	return true;
}

bool Leviathan::Graphics::CreateDefaultRenderView(){
	// create fully window spanning default view //

	// create matching SceneManager //
	if(!CreateCameraAndNodesForScene()){

		return false;
	}


	// create the actual viewport //
	float ViewWidth = 1.f;
	float ViewHeight = 1.f;
	float ViewLeft = (1.f-ViewWidth)*0.5f;
	float ViewTop = (1.f-ViewHeight)*0.5f;

	USHORT ZOrder = 100;

	MainViewport = AppDefinition->GetWindow()->addViewport(MainCamera, ZOrder, ViewLeft, ViewTop, ViewWidth, ViewHeight);

	// set default viewport colour //
	MainViewport->setBackgroundColour(Ogre::ColourValue(0.3f, 0.6f, 0.9f));

	// automatic updating //
	MainViewport->setAutoUpdated(true);

	// set aspect ratio to the same as the view port (this makes it look realistic) //
	MainCamera->setAspectRatio(MainViewport->getActualWidth()/(float)MainViewport->getActualHeight());

	// near and far clipping planes //
	MainCamera->setFOVy(Ogre::Radian(60.f*DEGREES_TO_RADIANS));
	// MadMarx tip (far/near > 2000 equals probles) //
	MainCamera->setNearClipDistance(0.3f);
	//MainCamera->setNearClipDistance(1.3f);
	//MainCamera->setFarClipDistance(2600.f);


	return true;
}

bool Leviathan::Graphics::CreateCameraAndNodesForScene(){
	// create scene manager //
	MainScene = ORoot->createSceneManager(Ogre::ST_INTERIOR, "MainSceneManager");

	// create camera //
	MainCamera = MainScene->createCamera("Camera01");

	// create node for camera and attach it //
	MainCameraNode = MainScene->getRootSceneNode()->createChildSceneNode("MainCameraNode");
	MainCameraNode->attachObject(MainCamera);



	return true;
}

void Leviathan::Graphics::CreateTestObject(){

	string CubeName = "DefaultTestCube";

	vector<Float3> positions(5);
	// create several meshes //
	for(int i = 0; i < 5; ++i){

		float offset = (float)(1+i*2-5);
		positions[i] = Float3(offset, offset, -14.f);
	}

	// create it and create instances //
	Engine::GetEngine()->GetObjectLoader()->CreateTestCubeToScene(MainScene, CubeName);

	Engine::GetEngine()->GetObjectLoader()->AddTestCubeToScenePositions(MainScene, positions, CubeName);

}

// ------------------------------------------- //
DLLEXPORT bool Leviathan::Graphics::Frame(int mspassed, ViewerCameraPos* camerapostouse, vector<BaseRenderable*> &objects){
	// update camera //
	camerapostouse->UpdatePos(mspassed);

	// set camera position //
	MainCameraNode->setPosition(camerapostouse->GetPosition());

	// convert rotation into a quaternion //
	const Float3& angles = camerapostouse->GetRotation();

	// create quaternion from quaternion rotations around each axis //
	Ogre::Quaternion rotq(Ogre::Degree(angles.Y), Ogre::Vector3::UNIT_X);
	Ogre::Quaternion rotyaw(Ogre::Degree(angles.X), Ogre::Vector3::UNIT_Y);
	Ogre::Quaternion rotroll(Ogre::Degree(angles.Z), Ogre::Vector3::UNIT_Z);

	rotq = rotyaw*rotq*rotroll;

	//// create point that the camera looks at and set it //
	//Float3 point = camerapostouse->GetPosition()+(Float3(-sin(angles.X*DEGREES_TO_RADIANS), sin(angles.Y*DEGREES_TO_RADIANS), 
	//	-cos(angles.X*DEGREES_TO_RADIANS))*10.f);

	//MainCamera->lookAt(point);

	MainCamera->setOrientation(rotq);

	// call rendering function //
	if(!Render(mspassed, objects)){

		return false;
	}

	// now we can render one frame //
	ORoot->renderOneFrame();

	return true;
}

bool Graphics::Render(int mspassed, vector<BaseRenderable*> &objects){

	// go through objects and call their render functions //
	for(size_t i = 0; i < objects.size(); i++){
		if(objects[i]->IsHidden())
			continue;
		DEBUG_BREAK;
		// objects should be smart enough to not to change common matrices
		//objects[i]->Render(this, mspassed, *CurrentPass.get()); 
	}

	DrawRenderActions();

	// we can now actually render the window //
	Ogre::RenderWindow* tmpwindow = AppDefinition->GetWindow();

	tmpwindow->update(false);
	// all automatically updated view ports are updated //

	// update special view ports //

	// finish rendering the main window //
	tmpwindow->swapBuffers(AppDefinition->GetVSync());

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
void Graphics::DrawRenderActions(/*RenderingPassInfo* pass*/){
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

					ComplainOnce::PrintWarningOnce(L"no rendering code", L"no rendering code"__WFILE__ L" func" __WFUNCTION__);
				}
			}
		}
	}
}


