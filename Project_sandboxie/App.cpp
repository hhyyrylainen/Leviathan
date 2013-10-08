// ------------------------------------ //
#ifndef SANDBOXIE_APP
#include "App.h"
#endif
#include "Entities\Objects\ViewerCameraPos.h"
#include "Entities\GameWorld.h"
#include "Entities\Objects\Prop.h"
//using namespace Leviathan;
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


	// setup world //
	shared_ptr<GameWorld> world1 = Engine::GetEngine()->CreateWorld();



	// camera //
	shared_ptr<ViewerCameraPos> MainCamera(new ViewerCameraPos());
	MainCamera->SetPos(Float3(0, 0, 0));

	// link world and camera to a window //
	GraphicalInputEntity* window1 = Engine::GetEngine()->GetWindowEntity();

	window1->LinkCamera(MainCamera);
	window1->LinkWorld(world1);
	// sound listening camera //
	MainCamera->BecomeSoundPerceiver();

	// link window input to camera //
	window1->GetInputController()->LinkReceiver(MainCamera.get());


	// load test objects //
	auto prop1holder = world1->GetObject(Engine::GetEngine()->GetObjectLoader()->LoadPropToWorld(L"Cube", world1.get()));

	if(prop1holder.get() != NULL){
		Entity::Prop* tmp = static_cast<Entity::Prop*>(prop1holder.get());
		// set position //

		tmp->SetPos(3.f, -1.f, -10.f);
	}

	
	// after loading reset time sensitive timers //
	Engine::GetEngine()->ResetPhysicsTime();
	


	//Float3 startpos(0, 300, 60);


	//string CubeName = "DefaultTestCube";

	//vector<Float3> positions(5);
	//// create several meshes //
	//for(int i = 0; i < 5; ++i){

	//	float offset = (float)(1+i*2-5);
	//	positions[i] = startpos+Float3(offset, offset, -14.f);
	//}

	//// create it and create instances //
	//Engine::GetEngine()->GetObjectLoader()->CreateTestCubeToScene(MainScene, CubeName);

	//Engine::GetEngine()->GetObjectLoader()->AddTestCubeToScenePositions(MainScene, positions, CubeName);

	//// create test model //
	//try{
	//	Ogre::Entity* ModelEntity = MainScene->createEntity("Cube.mesh");
	//	// casts shadows //
	//	ModelEntity->setCastShadows(true);

	//	// attach to new node //
	//	Ogre::SceneNode* mnode = MainScene->getRootSceneNode()->createChildSceneNode();

	//	mnode->attachObject(ModelEntity);
	//	// set position //
	//	mnode->setPosition(startpos+Float3(0.f, -2.f, -5.f));
	//}
	//catch(const Ogre::FileNotFoundException &e){

	//	Logger::Get()->Error(L"[EXCEPTION] "+Convert::StringToWstring(e.getFullDescription()));
	//}

	//// plane for bottom //
	//Ogre::Plane ground(Ogre::Vector3::UNIT_Y, Ogre::Vector3(startpos-Float3(0, 2, 0)));
	//Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
	//	ground, 300, 300, 20, 20, true, 1, 65, 65, Ogre::Vector3::UNIT_Z);

	//// platform that receives shadows //
	//Ogre::Entity* groundentity = MainScene->createEntity("GroundEntity", "ground");
	//MainScene->getRootSceneNode()->createChildSceneNode()->attachObject(groundentity);

	//// set nice stone material //
	//groundentity->setMaterialName("Material.001");


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

//LRESULT CALLBACK App::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	// handle creation message here //
//	if(message == WM_CREATE){
//
//		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
//		WindowPassData* WData = (WindowPassData *)pcs->lpCreateParams;
//
//
//		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(WData));
//
//		return 1;
//	}
//
//	WindowPassData* WData = reinterpret_cast<WindowPassData*>(static_cast<LONG_PTR>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA)));
//
//	// skip if empty data //
//	if(!WData){
//		return DefWindowProc(hwnd, message, wParam, lParam);
//	}
//
//	switch (message){
//	case WM_SIZE:
//		{
//			UINT width = LOWORD(lParam);
//			UINT height = HIWORD(lParam);
//
//			WData->Appinterface->OnResize(width, height);
//			return 0;
//		}
//	case WM_DISPLAYCHANGE:
//		{
//			InvalidateRect(hwnd, NULL, FALSE);
//			return 0;
//		}
//	case WM_MENUCHAR:
//		{
//			return MNC_CLOSE << 16;
//		}
//	case WM_QUIT: case WM_CLOSE: case WM_DESTROY:
//		{
//			// close the application //
//			WData->Appinterface->StartRelease();
//			// close passed data //
//			SAFE_DELETE(WData);
//			// set back so that it won't be used anymore //
//			::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(WData));
//
//			return 0;
//		}
//		break;
//	case WM_SETFOCUS:
//		{
//			WData->Appinterface->GainFocus();
//			return 0;
//		}
//	case WM_KILLFOCUS:
//		{
//			WData->Appinterface->LoseFocus();
//			return 0;
//		}
//	case WM_KEYDOWN:
//		switch(wParam){
//		case VK_ESCAPE:
//			{
//				WData->Appinterface->StartRelease();
//				return 0;
//			}
//		}
//	break;
//	}
//
//	// got here, tell engine to handle, or resort to default window procedure //
//	if(!WData->Appinterface->GetEngine()->HandleWindowCallBack(message, wParam, lParam))
//		return DefWindowProc(hwnd, message, wParam, lParam);
//
//	return 0;
//}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //