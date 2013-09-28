#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GRAPHICS
#include "Graphics.h"
#endif
#include "Application\AppDefine.h"
#include "Application\Application.h"
#include "Utility\ComplainOnce.h"
#include <boost\assign\list_of.hpp>
#include "OgreHardwarePixelBuffer.h"
#include <OgreMeshManager.h>
#include "GUI/OverlayMaster.h"
#include "GUI/FontManager.h"
#include <OgreManualObject.h>
#include <OgreFrameListener.h>
#include "TextureManager.h"
using namespace Leviathan;
using namespace Rendering;
// ------------------------------------ //


DLLEXPORT Leviathan::Graphics::Graphics() : Light(NULL), TextureKeeper(NULL), ORoot(nullptr), MainCamera(NULL), MainCameraNode(NULL), MainViewport(NULL)
	/*, Terrain(NULL)*/, Overlays(NULL), Fonts(NULL)
{
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
	if(!TextureKeeper->Init(FileSystem::GetTextureFolder())){

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

DLLEXPORT void Leviathan::Graphics::Release(){

	//SAFE_DELETE(Terrain);
	// release overlay //
	Overlays->Release();
	
	SAFE_DELETE(Fonts);


	SAFE_DELETE(Light);
	SAFE_RELEASEDEL(TextureKeeper);

	ORoot.reset();
	// delete listeners and overlay master memory //
	SAFE_DELETE(Overlays);

	Initialized = false;
}
// ------------------------------------------- //
bool Leviathan::Graphics::InitializeOgre(AppDef* appdef){

	Ogre::String ConfigFileName = "";
	Ogre::String PluginsFileName = "";

	Ogre::LogManager* logMgr = new Ogre::LogManager();

	OLog = Ogre::LogManager::getSingleton().createLog("LogOGRE.txt", true, true, false);
	OLog->setDebugOutputEnabled(true);
	OLog->setLogDetail(Ogre::LL_NORMAL);

	ORoot = unique_ptr<Ogre::Root>(new Ogre::Root(PluginsFileName, ConfigFileName, ""));


	vector<Ogre::String> PluginNames = boost::assign::list_of("RenderSystem_GL")/*("RenderSystem_Direct3D11")*/("Plugin_ParticleFX")
		("Plugin_CgProgramManager")("Plugin_OctreeSceneManager")/*("OgrePaging")("OgreTerrain")("OgreOverlay")*/;

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

	Ogre::RenderSystem* selectedrendersystem = RSystemList[0];

	Ogre::ConfigOptionMap& rconfig = selectedrendersystem->getConfigOptions();
	if(rconfig.find("RTT Preferred Mode") != rconfig.end()){
		// set to copy, can fix problems //
		// this causes spam on my setup and doesn't fix any issues
		//selectedrendersystem->setConfigOption("RTT Preferred Mode","Copy");
		//selectedrendersystem->setConfigOption("RTT Preferred Mode","FBO");
	}

	// for now just choose the first one in the list //
	ORoot->setRenderSystem(selectedrendersystem);

	ORoot->initialise(false, "", "");

	// register listener //
	ORoot->addFrameListener(this);

	// we can now ourselves create a window //
	const WindowDataDetails& WData = AppDefinition->GetWindowDetails();

	// get vsync (this is rather expensive so it is stored) //
	bool vsync = AppDefinition->GetVSync();

	// set some rendering specific parameters //
	Ogre::NameValuePairList WParams;

	// set anti aliasing //
	// temporary parameters for creating a graphics instance //
	int FSAA = 4;

	// get variables from engine configuration file //
	ObjectFileProcessor::LoadValueFromNamedVars<int>(AppDefinition->GetValues(), L"FSAA", FSAA, 4, true, L"Graphics: Init:");
	
	Ogre::String fsaastr = Convert::ToString(FSAA);

	WParams["FSAA"] = fsaastr;
	WParams["vsync"] = vsync ? "true": "false";

	Ogre::String wcaption = Convert::WstringToString(WData.Title);
	// quicker access to the window //
	Ogre::RenderWindow* tmpwindow = ORoot->createRenderWindow(wcaption, WData.Width, WData.Height, !WData.Windowed, &WParams);

	// create the actual window and store it at the same time //
	AppDefinition->SetRenderingWindow(new Window(tmpwindow, vsync));
	// apply style settings (mainly ICON) //
	WData.ApplyIconToHandle(AppDefinition->GetWindow()->GetHandle());
	AppDefinition->GetWindow()->GetOgreWindow()->setDeactivateOnFocusChange(false);

	Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
	Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);

	AppDefinition->GetWindow()->GetOgreWindow();

	// set the main window to be active //
	tmpwindow->setActive(true);

	// manual window updating //
	tmpwindow->setAutoUpdated(false);

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

	if(!CreateDefaultRenderView()){

		return false;
	}

	// load fonts before overlay //
	Fonts = new Rendering::FontManager();

	if(!InitializeOverlay()){

		Logger::Get()->Error(L"Graphics: Init: cannot initialize GUI rendering");
		return false;
	}

	// set loading paths //
	FileSystem::RegisterOGREResourceGroups();

	ConfigureTestRendering();

	// clear events that might have queued A LOT while starting up //
	ORoot->clearEventTimes();

	return true;
}
// ------------------------------------ //
void Leviathan::Graphics::ConfigureTestRendering(){

	CreateTestObject();

	// test light //
	Ogre::Light* TLight = MainScene->createLight("testlight");

	TLight->setType(Ogre::Light::LT_DIRECTIONAL);
	//TLight->setDirection(Float3(0.55f, -0.3f, 0.75f).Normalize());
	TLight->setDiffuseColour(0.98f, 1.f, 0.95f);
	TLight->setSpecularColour(1.f, 1.f, 1.f);

	Ogre::SceneNode* lnode = MainScene->getRootSceneNode()->createChildSceneNode();
	lnode->attachObject(TLight);

	Ogre::Quaternion quat;
	quat.FromAngleAxis(Ogre::Radian(1.f), Float3(0.55f, -0.3f, 0.75f));
	lnode->setOrientation(quat);

	// set scene ambient colour //
	MainScene->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f));

	// set sky dome //
	try{
		//MainScene->setSkyBox(true, "NiceDaySky.material");
		MainScene->setSkyBox(true, "NiceDaySky");
	}
	catch(const Ogre::InvalidParametersException &e){

		Logger::Get()->Error(L"[EXCEPTION] "+Convert::StringToWstring(e.getFullDescription()));
	}

	MainScene->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue(0.7f, 0.7f, 0.8f), 0, 4000, 10000);
	//MainScene->setFog(Ogre::FOG_NONE);

	//// create terrain //
	//Terrain = new WorldTerrain(TLight, MainScene, MainCamera);
}
// ------------------------------------ //
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

	MainViewport = AppDefinition->GetWindow()->GetOgreWindow()->addViewport(MainCamera, ZOrder, ViewLeft, ViewTop, ViewWidth, ViewHeight);

	// set default viewport colour //
	MainViewport->setBackgroundColour(Ogre::ColourValue(0.3f, 0.6f, 0.9f));

	// automatic updating //
	MainViewport->setAutoUpdated(true);

	float aspectratio = MainViewport->getActualWidth()/(float)MainViewport->getActualHeight();
	
	// set aspect ratio to the same as the view port (this makes it look realistic) //
	MainCamera->setAspectRatio(aspectratio);
	MainCamera->setAutoAspectRatio(true);

	// near and far clipping planes //
	MainCamera->setFOVy(Ogre::Radian(60.f*DEGREES_TO_RADIANS));
	// MadMarx tip (far/near > 2000 equals probles) //
	MainCamera->setNearClipDistance(0.1f);
	//MainCamera->setNearClipDistance(1.3f);
	MainCamera->setFarClipDistance(50000.f);

	if(ORoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE)){
		// enable infinite far clip distance if we can
		MainCamera->setFarClipDistance(0);   
	}


	return true;
}

bool Leviathan::Graphics::CreateCameraAndNodesForScene(){
	// create scene manager //
	MainScene = ORoot->createSceneManager(Ogre::ST_EXTERIOR_FAR, "MainSceneManager");

	// set scene shadow types //
	MainScene->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

	// create camera //
	MainCamera = MainScene->createCamera("Camera01");

	// create node for camera and attach it //
	MainCameraNode = MainScene->getRootSceneNode()->createChildSceneNode("MainCameraNode");
	MainCameraNode->attachObject(MainCamera);



	return true;
}

void Leviathan::Graphics::CreateTestObject(){


	Float3 startpos(0, 300, 60);


	string CubeName = "DefaultTestCube";

	vector<Float3> positions(5);
	// create several meshes //
	for(int i = 0; i < 5; ++i){

		float offset = (float)(1+i*2-5);
		positions[i] = startpos+Float3(offset, offset, -14.f);
	}

	// create it and create instances //
	Engine::GetEngine()->GetObjectLoader()->CreateTestCubeToScene(MainScene, CubeName);

	Engine::GetEngine()->GetObjectLoader()->AddTestCubeToScenePositions(MainScene, positions, CubeName);

	// create test model //
	try{
		Ogre::Entity* ModelEntity = MainScene->createEntity("Cube.mesh");
		// casts shadows //
		ModelEntity->setCastShadows(true);

		// attach to new node //
		Ogre::SceneNode* mnode = MainScene->getRootSceneNode()->createChildSceneNode();

		mnode->attachObject(ModelEntity);
		// set position //
		mnode->setPosition(startpos+Float3(0.f, -2.f, -5.f));
	}
	catch(const Ogre::FileNotFoundException &e){

		Logger::Get()->Error(L"[EXCEPTION] "+Convert::StringToWstring(e.getFullDescription()));
	}

	// plane for bottom //
	Ogre::Plane ground(Ogre::Vector3::UNIT_Y, Ogre::Vector3(startpos-Float3(0, 2, 0)));
	Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		ground, 300, 300, 20, 20, true, 1, 65, 65, Ogre::Vector3::UNIT_Z);

	// platform that receives shadows //
	Ogre::Entity* groundentity = MainScene->createEntity("GroundEntity", "ground");
	MainScene->getRootSceneNode()->createChildSceneNode()->attachObject(groundentity);

	// set nice stone material //
	groundentity->setMaterialName("Material.001");


	// test render to texture //
	Ogre::TextureManager& tmptextures = Ogre::TextureManager::getSingleton();

	Ogre::String RttTextureName = "MainSceneTestRTT";
	Ogre::String RttMaterialName = "TestRttMaterial";

	Ogre::TexturePtr TextureWithRtt = tmptextures.createManual(RttTextureName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Ogre::TEX_TYPE_2D, 1024, 1024, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET, 0, true, 2);

	// create the drawing surface and a camera for it //
	Ogre::HardwarePixelBufferSharedPtr RttTextureBuffer = TextureWithRtt->getBuffer();
	Ogre::RenderTexture* RttTextureRenderTarget = RttTextureBuffer->getRenderTarget();
	// we don't want to manually update this //
	RttTextureRenderTarget->setAutoUpdated(true);
	// new camera to match aspect ration, could use existing //
	Ogre::Camera * RttTextureCamera = MainScene->createCamera("TestRttCamera");
	// same rules as in all cameras //
	RttTextureCamera->setNearClipDistance(1.5f);
	RttTextureCamera->setFarClipDistance(3000.0f);
	// aspect ration from the texture //
	RttTextureCamera->setAspectRatio(1.0f);

	Ogre::SceneNode* TestRttCameraNode = MainScene->getRootSceneNode()->createChildSceneNode();
	TestRttCameraNode->attachObject(RttTextureCamera);

	TestRttCameraNode->setPosition(startpos+Float3(2.f, 1.f, -6.f));
	TestRttCameraNode->lookAt((Ogre::Vector3)startpos+Ogre::Vector3(0.f, 0.f, -10.0f), Ogre::Node::TS_WORLD);

	Ogre::Viewport* TestRttTextureViewport1 = RttTextureRenderTarget->addViewport(RttTextureCamera, 100, 0.f, 0.f, 1.f, 1.f);
	// nor manually update this //
	TestRttTextureViewport1->setAutoUpdated(true);
	TestRttTextureViewport1->setBackgroundColour(Ogre::ColourValue(0.f,0.f,1.f,1.f));
	// we definitely don't want overlays over this //
	TestRttTextureViewport1->setOverlaysEnabled(false);


	// material with the texture //
	Ogre::MaterialManager& tmpmaterialmanager = Ogre::MaterialManager::getSingleton();
	Ogre::MaterialPtr TestRttMaterial = tmpmaterialmanager.create(RttMaterialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique * technique = TestRttMaterial->getTechnique(0);
	Ogre::Pass* pass = technique->getPass(0);
	Ogre::TextureUnitState* textureunit = pass->createTextureUnitState();
	textureunit->setTextureName(RttTextureName);
	textureunit->setNumMipmaps(0);
	textureunit->setTextureFiltering(Ogre::TFO_BILINEAR);

	// object that has the render target on it //
	// entity that isn't visible to the rtt camera (would block it's vision) //
	Ogre::Entity* RttDisplayer = MainScene->createEntity("RttQuad");
	RttDisplayer->setMaterialName(RttMaterialName);
	RttDisplayer->setCastShadows(true);
	// how is this visible only by the first camera and not the rtt camera //
	Ogre::SceneNode* lVisibleOnlyByFirstCam = MainScene->getRootSceneNode()->createChildSceneNode();
	lVisibleOnlyByFirstCam->attachObject(RttDisplayer);
	lVisibleOnlyByFirstCam->setPosition(startpos+Float3(2.0f, -1.f, -5.5f));


	// add an extra light to light the rtt quad //
	Ogre::Light* rttplight = MainScene->createLight("rttlighter");
	rttplight->setType(Ogre::Light::LT_POINT);
	rttplight->setPosition(Ogre::Vector3(startpos+Float3(6.f, 2.f, 0.f)));
	rttplight->setDiffuseColour(Ogre::ColourValue::White);
	rttplight->setSpecularColour(Ogre::ColourValue::ZERO);

	//Light::setAttenuation

	// set to node //
	MainScene->getRootSceneNode()->createChildSceneNode()->attachObject(rttplight);
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

	MainCamera->setOrientation(rotq);

	// call rendering function //
	if(!Render(mspassed, objects)){

		return false;
	}

	// now we can render one frame //
	ORoot->renderOneFrame();

	return true;
}

bool Leviathan::Graphics::frameRenderingQueued(const Ogre::FrameEvent& evt){
	// we can update GUI inputs now //

	//Engine::GetEngine()->GetDefinition()->GetWindow()->GatherInput();


	return true;
}

DLLEXPORT void Leviathan::Graphics::SaveScreenShot(const string &filename){
	// uses render target's capability to save it's contents //
	AppDefinition->GetWindow()->GetOgreWindow()->writeContentsToTimestampedFile(filename, "_window1.png");
}

bool Leviathan::Graphics::InitializeOverlay(){
	// create overlay manager //
	Overlays = new Rendering::OverlayMaster(MainScene, MainViewport, Fonts);
	// if no exceptions were thrown it succeeded //
	return true;
}

bool Graphics::Render(int mspassed, vector<BaseRenderable*> &objects){

	// go through objects and call their render functions //
	for(size_t i = 0; i < objects.size(); i++){
		if(objects[i]->IsHidden())
			continue;
		DEBUG_BREAK;
	}

	// we can now actually render the window //
	Ogre::RenderWindow* tmpwindow = AppDefinition->GetWindow()->GetOgreWindow();


	// cancel actual rendering if window closed //
	if(tmpwindow->isClosed()){

		Logger::Get()->Warning(L"Graphics: Render: skipping render due to window being closed");
		return false;
	}

	tmpwindow->update(false);
	// all automatically updated view ports are updated //

	// update special view ports //

	// finish rendering the main window //
	tmpwindow->swapBuffers(AppDefinition->GetWindow()->GetVsync());

	return true;
}
// ------------------------------------------- //


