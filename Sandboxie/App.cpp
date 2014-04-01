// ------------------------------------ //
#ifndef SANDBOXIE_APP
#include "App.h"
#endif
#include "Entities/Objects/ViewerCameraPos.h"
#include "Entities/GameWorld.h"
#include "Entities/Objects/Prop.h"
#include "Entities/Objects/Brush.h"
#include "Rendering/GraphicalInputEntity.h"
#include "GUI/GuiManager.h"
#include "Engine.h"
#include "Handlers/ObjectLoader.h"
using namespace Leviathan;
using namespace SandBoxie;
// ------------------------------------ //
App::App(){

}
// ------------------------------------ //
void App::CustomizeEnginePostLoad(){
	// load GUI documents //

	Gui::GuiManager* manager = Engine::GetEngine()->GetWindowEntity()->GetGUI();

	manager->LoadGUIFile(FileSystem::GetScriptsFolder()+L"MainGui.txt");

	manager->SetMouseFile(FileSystem::GetScriptsFolder()+L"cursor.rml");


	shared_ptr<ViewerCameraPos> MainCamera(new ViewerCameraPos());

	// setup world //
	shared_ptr<GameWorld> world1 = Engine::GetEngine()->CreateWorld(Engine::Get()->GetWindowEntity(), MainCamera);

	ObjectLoader* loader = Engine::GetEngine()->GetObjectLoader();


	// camera //
	MainCamera->SetPos(Float3(-1.f, 2.5f, 10.f));

	// link world and camera to a window //
	GraphicalInputEntity* window1 = Engine::GetEngine()->GetWindowEntity();

	window1->LinkObjects(MainCamera, world1);
	// sound listening camera //
	MainCamera->BecomeSoundPerceiver();

	// link window input to camera //
	window1->GetInputController()->LinkReceiver(MainCamera.get());


	// load test objects //
	Entity::Prop* tmpprop;
	auto prop1holder = world1->GetWorldObject(loader->LoadPropToWorld(world1.get(), L"Cube", &tmpprop));

	if(prop1holder.get() != NULL){
		// set position //
		tmpprop->SetPosComponents(-2.f, 4.f, 0.f);
	}

	Entity::Brush* tmp;

	// create brush for the block to fall onto //
	auto brush1 = world1->GetWorldObject(loader->LoadBrushToWorld(world1.get(), "Material.001", Float3(14.f, 1.f, 14.f), 0.f, &tmp));

	if(brush1.get() != NULL){
		// set position //
		tmp->SetPosComponents(0.f, -0.5f, 0.f);
	}


	brush1 = world1->GetWorldObject(loader->LoadBrushToWorld(world1.get(), "Material.001", Float3(2.f, 2.f, 2.f), 100.f, &tmp));

	if(brush1.get() != NULL){
		// set position //
		tmp->SetPosComponents(1.f, 3.f, 0.f);
	}

	brush1 = world1->GetWorldObject(loader->LoadBrushToWorld(world1.get(), "Material.001", Float3(2.f, 2.f, 2.f), 100.f, &tmp));

	if(brush1.get() != NULL){
		// set position //
		tmp->SetPosComponents(0.f, 7.f, 0.f);
	}

	brush1 = world1->GetWorldObject(loader->LoadBrushToWorld(world1.get(), "Material.001", Float3(2.f, 2.f, 2.f), 100.f, &tmp));

	if(brush1.get() != NULL){
		// set position //
		tmp->SetPosComponents(-1.f, 10.f, -1.f);
	}
	
	// after loading reset time sensitive timers //
	Engine::GetEngine()->ResetPhysicsTime();

	// we want to capture mouse to main window //
	Engine::GetEngine()->GetWindowEntity()->SetMouseCapture(true);

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

	//TestRttCameraNode->setPosition(startpos+Float3(2.f, 1.f, -6.f));
	//TestRttCameraNode->lookAt((Ogre::Vector3)startpos+Ogre::Vector3(0.f, 0.f, -10.0f), Ogre::Node::TS_WORLD);

	//Ogre::Viewport* TestRttTextureViewport1 = RttTextureRenderTarget->addViewport(RttTextureCamera, 100, 0.f, 0.f, 1.f, 1.f);
	//// nor manually update this //
	//TestRttTextureViewport1->setAutoUpdated(true);
	//TestRttTextureViewport1->setBackgroundColour(Ogre::ColourValue(0.f,0.f,1.f,1.f));
	//// we definitely don't want overlays over this //
	//TestRttTextureViewport1->setOverlaysEnabled(false);


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
	//lVisibleOnlyByFirstCam->setPosition(startpos+Float3(2.0f, -1.f, -5.5f));


	//// add an extra light to light the rtt quad //
	//Ogre::Light* rttplight = MainScene->createLight("rttlighter");
	//rttplight->setType(Ogre::Light::LT_POINT);
	//rttplight->setPosition(Ogre::Vector3(startpos+Float3(6.f, 2.f, 0.f)));
	//rttplight->setDiffuseColour(Ogre::ColourValue::White);
	//rttplight->setSpecularColour(Ogre::ColourValue::ZERO);

	////Light::setAttenuation

	//// set to node //
	//MainScene->getRootSceneNode()->createChildSceneNode()->attachObject(rttplight);
}
// ------------------------------------ //
std::wstring SandBoxie::App::GenerateWindowTitle(){
	return wstring(L"Sandboxie version " GAME_VERSIONS L" Leviathan " LEVIATHAN_VERSIONS);
}

// ------------------------------------ //
void SandBoxie::App::CheckGameConfigurationVariables(GameConfiguration* configobj){

}

void SandBoxie::App::CheckGameKeyConfigVariables(KeyConfiguration* keyconfigobj){

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //