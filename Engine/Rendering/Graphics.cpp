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
#include "OgreHardwarePixelBuffer.h"

DLLEXPORT Leviathan::Graphics::Graphics() : Light(NULL), TextureKeeper(NULL), ORoot(nullptr), MainCamera(NULL), MainCameraNode(NULL), MainViewport(NULL), 
	_TerrainGlobalSettings(NULL), _TerrainGroup(NULL)
{
	GuiSmooth = 5;

	_TerrainImported = false;
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

	OGRE_DELETE _TerrainGroup;
	OGRE_DELETE _TerrainGlobalSettings;

	Initialized = false;
}
// ------------------------------------------- //
bool Leviathan::Graphics::InitializeOgre(AppDef* appdef){

	Ogre::String ConfigFileName = "";
	Ogre::String PluginsFileName = "";

	Ogre::String OgreLogName = "LogOGRE.txt";

	ORoot = unique_ptr<Ogre::Root>(new Ogre::Root(PluginsFileName, ConfigFileName, OgreLogName));


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

	if(!CreateDefaultRenderView()){

		return false;
	}

	// set the main window to be active //
	tmpwindow->setActive(true);

	// manual window updating //
	tmpwindow->setAutoUpdated(false);


	// set loading paths //
	FileSystem::RegisterOGREResourceGroups();

	// set scene shadow types //
	MainScene->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);


	//CreateTestObject();

	// create test model //
	//try{
	//	Ogre::Entity* ModelEntity = MainScene->createEntity("Cube.mesh");
	//	// casts shadows //
	//	ModelEntity->setCastShadows(true);

	//	// attach to new node //
	//	Ogre::SceneNode* mnode = MainScene->getRootSceneNode()->createChildSceneNode();

	//	mnode->attachObject(ModelEntity);
	//	// set position //
	//	mnode->setPosition(0.f, -2.f, -5.f);
	//}
	//catch(const Ogre::FileNotFoundException &e){

	//	Logger::Get()->Error(L"[EXCEPTION] "+Convert::StringToWstring(e.getFullDescription()));
	//}

	// platform that receives shadows //
	//try{
	//	Ogre::Entity* ModelEntity = MainScene->createEntity("Cube.002.mesh");
	//	// casts shadows //
	//	ModelEntity->setCastShadows(true);

	//	// attach to new node //
	//	Ogre::SceneNode* mnode = MainScene->getRootSceneNode()->createChildSceneNode();

	//	mnode->attachObject(ModelEntity);
	//	// set position //
	//	mnode->setPosition(0.f, -3.f, -5.f);
	//}
	//catch(const Ogre::FileNotFoundException &e){

	//	Logger::Get()->Error(L"[EXCEPTION] "+Convert::StringToWstring(e.getFullDescription()));
	//}


	//// test render to texture //
	//Ogre::TextureManager& tmptextures = Ogre::TextureManager::getSingleton();

	//Ogre::String RttTextureName = "MainSceneTestRTT";
	//Ogre::String RttMaterialName = "TestRttMaterial";

	//Ogre::TexturePtr TextureWithRtt = tmptextures.createManual(RttTextureName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
	//	Ogre::TEX_TYPE_2D, 1024, 1024, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET, 0, true, 2);

	//// create the drawing surface and a camera for it //
	//Ogre::HardwarePixelBufferSharedPtr RttTextureBuffer = TextureWithRtt->getBuffer();
	//Ogre::RenderTexture* RttTextureRenderTarget = RttTextureBuffer->getRenderTarget();
	//// we don't want to manually update this //
	//RttTextureRenderTarget->setAutoUpdated(true);
	//// new camera to match aspect ration, could use existing //
	//Ogre::Camera * RttTextureCamera = MainScene->createCamera("TestRttCamera");
	//// same rules as in all cameras //
	//RttTextureCamera->setNearClipDistance(1.5f);
	//RttTextureCamera->setFarClipDistance(3000.0f);
	//// aspect ration from the texture //
	//RttTextureCamera->setAspectRatio(1.0f);

	//Ogre::SceneNode* TestRttCameraNode = MainScene->getRootSceneNode()->createChildSceneNode();
	//TestRttCameraNode->attachObject(RttTextureCamera);

	//TestRttCameraNode->setPosition(2.f, 1.f, -6.f);
	//TestRttCameraNode->lookAt(Ogre::Vector3(0.f, 0.f, -10.0f), Ogre::Node::TS_WORLD);

	//Ogre::Viewport* TestRttTextureViewport1 = RttTextureRenderTarget->addViewport(RttTextureCamera, 100, 0.f, 0.f, 1.f, 1.f);
	//// nor manually update this //
	//TestRttTextureViewport1->setAutoUpdated(true);
	//TestRttTextureViewport1->setBackgroundColour(Ogre::ColourValue(0.f,0.f,1.f,1.f));


	//// material with the texture //
	//Ogre::MaterialManager& tmpmaterialmanager = Ogre::MaterialManager::getSingleton();
	//Ogre::MaterialPtr TestRttMaterial = tmpmaterialmanager.create(RttMaterialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	//Ogre::Technique * technique = TestRttMaterial->getTechnique(0);
	//Ogre::Pass* pass = technique->getPass(0);
	//Ogre::TextureUnitState* textureunit = pass->createTextureUnitState();
	//textureunit->setTextureName(RttTextureName);
	//textureunit->setNumMipmaps(0);
	//textureunit->setTextureFiltering(Ogre::TFO_BILINEAR);

	//// object that has the render target on it //
	//// entity that isn't visible to the rtt camera (would block it's vision) //
	//Ogre::Entity* RttDisplayer = MainScene->createEntity("RttQuad");
	//RttDisplayer->setMaterialName(RttMaterialName);
	//RttDisplayer->setCastShadows(true);
	//// how is this visible only by the first camera and not the rtt camera //
	//Ogre::SceneNode* lVisibleOnlyByFirstCam = MainScene->getRootSceneNode()->createChildSceneNode();
	//lVisibleOnlyByFirstCam->attachObject(RttDisplayer);
	//lVisibleOnlyByFirstCam->setPosition(2.0f, -1.f, -5.5f);


	// test light //
	Ogre::Light* TLight = MainScene->createLight();

	TLight->setType(Ogre::Light::LT_DIRECTIONAL);
	TLight->setDirection(Float3(0.55f, -0.3f, 0.75f).Normalize());
	TLight->setDiffuseColour(0.98f, 1.f, 0.95f);
	TLight->setSpecularColour(1.f, 1.f, 1.f);

	Ogre::SceneNode* lnode = MainScene->getRootSceneNode()->createChildSceneNode();
	lnode->attachObject(TLight);
	//lnode->setOrientation(Float4(1.f, 0.5f, -0.3f, 0.4f).Normalize());

	// set scene ambient colour //
	MainScene->setAmbientLight(Ogre::ColourValue(0.2f, 0.2f, 0.2f, 1.f));

	// set sky dome //
	try{
		//MainScene->setSkyBox(true, "NiceDaySky.material");
		MainScene->setSkyBox(true, "NiceDaySky");
	}
	catch(const Ogre::InvalidParametersException &e){

		Logger::Get()->Error(L"[EXCEPTION] "+Convert::StringToWstring(e.getFullDescription()));
	}

	// test terrain //
	_TerrainGlobalSettings = OGRE_NEW Ogre::TerrainGlobalOptions();
	_TerrainGroup = OGRE_NEW Ogre::TerrainGroup(MainScene, Ogre::Terrain::ALIGN_X_Z, 513, 12000.f);

	// data caching file names //
	_TerrainGroup->setFilenameConvention("TestTerrainCache", ".dat");
	//_TerrainGroup->setResourceGroup("Terrain");
	_TerrainGroup->setResourceGroup("General");
	_TerrainGroup->setOrigin(Ogre::Vector3::ZERO);
	_TerrainGroup->setAutoUpdateLod(Ogre::TerrainAutoUpdateLodFactory::getAutoUpdateLod(Ogre::BY_DISTANCE) );

	TERRAIN_ConfigureTerrainDefaults(TLight);

	for(long x = 0;  x <= 0; x++){
		for(long y = 0; y <= 0; y++){
			TERRAIN_DefineTerrainAt(x, y);
		}
	}

	// we want all terrain to be loaded now //
	_TerrainGroup->loadAllTerrains(true);

	// if terrain has just been imported from generation (or image) we need to calculate blend maps //
	_TerrainImported = true;
	if(_TerrainImported){
		Ogre::TerrainGroup::TerrainIterator itr = _TerrainGroup->getTerrainIterator();
		// loop all elements and use blend map creation function //
		while(itr.hasMoreElements()){
			// get direct ptr to the terrain //
			Ogre::Terrain* tmp = itr.getNext()->instance;
			TERRAIN_InitBlendMaps(tmp);
			// re-calculate light maps //
			tmp->dirtyLightmap();
			tmp->update(true);
		}
	}

	// update the terrain objects //
	//_TerrainGroup->update(true);


	// cleanup after terrain generation //
	//_TerrainGroup->freeTemporaryResources();


	// clear events that might have queued A LOT while starting up //
	ORoot->clearEventTimes();

	return true;
}
// ------------------------------------ //
void Leviathan::Graphics::TERRAIN_ConfigureTerrainDefaults(Ogre::Light* light){
	// configure global terrain options //
	_TerrainGlobalSettings->setMaxPixelError(8);

	_TerrainGlobalSettings->setCompositeMapDistance(32000);
	_TerrainGlobalSettings->setSkirtSize(10);
	// TODO: this should be removed
	_TerrainGlobalSettings->setCastsDynamicShadows(false);

	// textures
	Ogre::TerrainMaterialGeneratorA::SM2Profile* mProfile = static_cast<Ogre::TerrainMaterialGeneratorA::SM2Profile*>(_TerrainGlobalSettings->
		getDefaultMaterialGenerator()->getActiveProfile());
	mProfile->setLayerParallaxMappingEnabled(false);
	mProfile->setLayerSpecularMappingEnabled(true);

	// non-realtime lighting data //
	_TerrainGlobalSettings->setLightMapDirection(light->getDerivedDirection());
	_TerrainGlobalSettings->setCompositeMapAmbient(MainScene->getAmbientLight());
	_TerrainGlobalSettings->setCompositeMapDiffuse(light->getDiffuseColour());

	_TerrainGlobalSettings->setLayerBlendMapSize(4096);
	_TerrainGlobalSettings->setLightMapSize(1024);


	// configure import settings from an image //
	Ogre::Terrain::ImportData& defaultimport = _TerrainGroup->getDefaultImportSettings();
	defaultimport.terrainSize = 513;
	defaultimport.worldSize = 12000.f;
	// image isn't pure floats, needs scaling //
	defaultimport.inputScale = 600;
	defaultimport.minBatchSize = 33;
	defaultimport.maxBatchSize = 65;

	// terrain textures //
	defaultimport.layerList.resize(3);
	defaultimport.layerList[0].worldSize = 100;
	defaultimport.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
	defaultimport.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");
	defaultimport.layerList[1].worldSize = 30;
	defaultimport.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
	defaultimport.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
	defaultimport.layerList[2].worldSize = 200;
	defaultimport.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
	defaultimport.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");
}

void Leviathan::Graphics::TERRAIN_DefineTerrainAt(long x, long y){
	// first get the name of the file that could contain the terrain data//
	Ogre::String filename = _TerrainGroup->generateFilename(x, y);

	// check does the file exist //
	if(Ogre::ResourceGroupManager::getSingleton().resourceExists(_TerrainGroup->getResourceGroup(), filename)){
		// has cached results //
		_TerrainGroup->defineTerrain(x, y);

	} else {
		// needs to generate new //
		Ogre::Image img;
		TERRAIN_GetTerrainImage(x % 2 != 0, y % 2 != 0, img);
		_TerrainGroup->defineTerrain(x, y, &img);
		_TerrainImported = true;
	}
	//// occlusion maps //

}

void Leviathan::Graphics::TERRAIN_GetTerrainImage(bool flipx, bool flipy, Ogre::Image &img){
	// load the terrain height map image and possibly flip it //
	img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	if(flipx)
		img.flipAroundY();
	if(flipy)
		img.flipAroundX();
}

void Leviathan::Graphics::TERRAIN_InitBlendMaps(Ogre::Terrain* terrain){
	// see defaultimport.layerList for reference to different layers //

	// use same fades as in ogre tutorial //
	Ogre::TerrainLayerBlendMap* BlendMap0 = terrain->getLayerBlendMap(1);
	Ogre::TerrainLayerBlendMap* BlendMap1 = terrain->getLayerBlendMap(2);

	// start and fade out heights //
	Ogre::Real minheight0 = 70;
	Ogre::Real fadedist0 = 40;
	Ogre::Real minheight1 = 70;
	Ogre::Real fadedist1 = 15;

	// direct blend map blend pointers //
	float* blend0 = BlendMap0->getBlendPointer();
	float* blend1 = BlendMap1->getBlendPointer();
	// loop through all and calculate blends //
	for(Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); y++){
		for(Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); x++){

			Ogre::Real tx, ty;

			BlendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
			Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
			// calculate blend value //
			Ogre::Real val = (height-minheight0)/fadedist0;
			val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
			// set //
			*blend0++ = val;

			// calculate second blend //
			val = (height-minheight1)/fadedist1;
			val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
			// set //
			*blend1++ = val;
		}
	}

	BlendMap0->dirty();
	BlendMap1->dirty();
	BlendMap0->update();
	BlendMap1->update();
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

	}

	DrawRenderActions();

	// we can now actually render the window //
	Ogre::RenderWindow* tmpwindow = AppDefinition->GetWindow()->GetOgreWindow();

	tmpwindow->update(false);
	// all automatically updated view ports are updated //

	// update special view ports //

	// finish rendering the main window //
	tmpwindow->swapBuffers(AppDefinition->GetWindow()->GetVsync());

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


