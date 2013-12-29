#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GRAPHICS
#include "Graphics.h"
#endif
#include "Application/AppDefine.h"
#include <OgreMeshManager.h>
#include "GUI/FontManager.h"
#include <OgreFrameListener.h>
#include "FileSystem.h"
#include <boost/assign/list_of.hpp>
#include "Engine.h"
using namespace Leviathan;
using namespace Rendering;
// ------------------------------------ //


DLLEXPORT Leviathan::Graphics::Graphics() : ORoot(nullptr), Fonts(NULL), Overlays(NULL)
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


	Initialized = true;
	return true;
}

DLLEXPORT void Leviathan::Graphics::Release(){

	SAFE_DELETE(Fonts);

	ORoot.reset();

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


	vector<Ogre::String> PluginNames = boost::assign::list_of("RenderSystem_GL")
#ifdef _WIN32
	("RenderSystem_Direct3D11")
#endif
		("Plugin_ParticleFX")("Plugin_CgProgramManager")("Plugin_OctreeSceneManager")/*("OgrePaging")("OgreTerrain")("OgreOverlay")*/;

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

	// Create the regular expression it must match //
	string rendersystemname;

	ObjectFileProcessor::LoadValueFromNamedVars<string>(appdef->GetValues(), L"RenderSystemName", rendersystemname, "OpenGL", true,
		L"Graphics: Init: no selected render system,");

    boost::regex rendersystemnameregex(rendersystemname, boost::regex_constants::icase);
	Ogre::RenderSystem* selectedrendersystem = NULL;

	// Choose the right render system //
	for(size_t i = 0; i < RSystemList.size(); i++){

		const Ogre::String& rsystemname = RSystemList[i]->getName();

		if(boost::regex_search(rsystemname, rendersystemnameregex)){

			// Matched //
			selectedrendersystem = RSystemList[i];
			break;
		}
	}

	if(!selectedrendersystem){
		// Select the first one since none matched //
		Logger::Get()->Warning(L"Graphics: Init: no render system matched regex, choosing default: "+Convert::StringToWstring(RSystemList[0]->getName()));
		selectedrendersystem = RSystemList[0];
	}

	// TODO: add device selecting feature //

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

	Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
	Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

	// load fonts before overlay //
	Fonts = new Rendering::FontManager();



	if(!InitializeOverlay()){
		return false;
	}

	// clear events that might have queued A LOT while starting up //
	ORoot->clearEventTimes();

	return true;
}
// ------------------------------------------- //
DLLEXPORT bool Leviathan::Graphics::Frame(){

	// all windows should already be updated //
	return ORoot->renderOneFrame();
}

bool Leviathan::Graphics::frameRenderingQueued(const Ogre::FrameEvent& evt){

	// simulate physics for next frame //
	Engine::GetEngine()->PhysicsUpdate();


	return true;
}

bool Leviathan::Graphics::InitializeOverlay(){
	Overlays = new Rendering::OverlayMaster();
	return true;
}
// ------------------------------------------- //


