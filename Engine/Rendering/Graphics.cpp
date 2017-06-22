// ------------------------------------ //
#include "Graphics.h"

#include "Application/AppDefine.h"
#include "Application/GameConfiguration.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GUI/FontManager.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "OgreLogManager.h"
#include "OgreMaterialManager.h"
#include "OgreRoot.h"
#include "OgreTextureManager.h"
#include "Threading/ThreadingManager.h"
#include <OgreFrameListener.h>
#include <OgreMeshManager.h>
#include <boost/assign/list_of.hpp>
#include <regex>
#include <future>

#include <SDL.h>
using namespace Leviathan;
using namespace Rendering;
using namespace std;
// ------------------------------------ //
#define OGRE_ALLOW_USEFULLOUTPUT

DLLEXPORT Leviathan::Graphics::Graphics(){
    
	Staticaccess = this;
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

		Logger::Get()->Error("Graphics: Init: failed to create ogre renderer");
		return false;
	}

	Initialized = true;
	return true;
}

DLLEXPORT void Leviathan::Graphics::Release(){

	Fonts.reset();

	ORoot.reset();

    if(Initialized){

        SDL_Quit();
    }

	Initialized = false;
}
// ------------------------------------------- //
bool Leviathan::Graphics::InitializeOgre(AppDef* appdef){

#ifndef LEVIATHAN_USING_SDL2
#error SDL2 required for Graphics.h but it is disabled
#endif
    
    // Startup SDL //
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0){

        LOG_ERROR("SDL init failed, error: " + std::string(SDL_GetError()));
        return false;
    }

    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
    

	Ogre::String ConfigFileName = "";
	Ogre::String PluginsFileName = "";

	Ogre::LogManager* logMgr = new Ogre::LogManager();

	// Could also use the singleton access method here //
	string ogrelogfile = appdef->GetLogFile() + "LogOGRE.txt";

	OLog = logMgr->createLog(ogrelogfile, true, true, false);
	OLog->setDebugOutputEnabled(true);
    
#ifdef OGRE_ALLOW_USEFULLOUTPUT

	bool usebore = false;
	{
		// Check if we want it //
		GAMECONFIGURATION_GET_VARIABLEACCESS(variables);

		if(variables)
			variables->GetValueAndConvertTo<bool>("OgreBoreMe", usebore);
	}

	if (usebore){
		OLog->setLogDetail(Ogre::LL_BOREME);
        
	} else {
        
		OLog->setLogDetail(Ogre::LL_NORMAL);
	}
#else
	OLog->setLogDetail(Ogre::LL_NORMAL);
#endif // OGRE_USEFULLOUTPUT


	ORoot = std::unique_ptr<Ogre::Root>(new Ogre::Root(PluginsFileName, ConfigFileName, ""));

	// Still waiting for the GL3Plus render system to become usable... //
	vector<Ogre::String> PluginNames = { "RenderSystem_GL"/*3Plus")*/,
#ifdef _WIN32
                                     #ifndef LEVIATHAN_USING_SDL2
		("RenderSystem_Direct3D11"),
                                     #endif
#endif
		("Plugin_ParticleFX")
            };
		// This seems no longer be available //
		/*("Plugin_CgProgramManager")*/
		/*("OgrePaging")("OgreTerrain")("OgreOverlay")*/;

	Ogre::String currentplugin = "";

	try{

        for(auto Iter = PluginNames.begin(); Iter != PluginNames.end(); Iter++){

            currentplugin = *Iter;

            // append "_d" if in debug mode //
#ifdef _DEBUG
            Iter->append("_d");
#endif // _DEBUG
            // load //
            ORoot->loadPlugin(*Iter);
		}

	}
	catch (const Ogre::InternalErrorException &e){

		Logger::Get()->Error("Graphics: init: failed to load ogre plugin (\""+currentplugin+
            "\"), exception: "+string(e.what()));
		return false;
	}


	// Choose proper render system //
	const Ogre::RenderSystemList& RSystemList = ORoot->getAvailableRenderers();

	if(RSystemList.size() == 0){
		// no render systems found //

		Logger::Get()->Error("Graphics: InitializeOgre: no render systems found");
		return false;
	}

	// Create the regular expression it must match //
	string rendersystemname;

	ObjectFileProcessor::LoadValueFromNamedVars<string>(appdef->GetValues(),
        "RenderSystemName", rendersystemname, "OpenGL.*",
        Logger::Get(), "Graphics: Init: no selected render system,");

	regex rendersystemnameregex(rendersystemname, regex_constants::ECMAScript |
        regex_constants::icase);

    Logger::Get()->Info("Graphics: preferred rendering system: \""+rendersystemname+"\"");
    
	Ogre::RenderSystem* selectedrendersystem = NULL;

	// Choose the right render system //
	for(size_t i = 0; i < RSystemList.size(); i++){

		const Ogre::String& rsystemname = RSystemList[i]->getName();

		if(regex_search(rsystemname, rendersystemnameregex)){

			// Matched //
			selectedrendersystem = RSystemList[i];
			break;
		}
	}

	if(!selectedrendersystem){
		// Select the first one since none matched //
		Logger::Get()->Warning("Graphics: Init: no render system matched regex, "
            "choosing default: " + RSystemList[0]->getName());
        
		selectedrendersystem = RSystemList[0];
	}

	// \todo add device selecting feature //

	Ogre::ConfigOptionMap& rconfig = selectedrendersystem->getConfigOptions();
	if(rconfig.find("RTT Preferred Mode") != rconfig.end()){
		// set to copy, can fix problems //
		// this causes spam on my setup and doesn't fix any issues
		//selectedrendersystem->setConfigOption("RTT Preferred Mode","Copy");
		//selectedrendersystem->setConfigOption("RTT Preferred Mode","FBO");
	}

	ORoot->setRenderSystem(selectedrendersystem);

	ORoot->initialise(false, "", "");

	// register listener //
	ORoot->addFrameListener(this);

    // TODO: Ogre HLMS needs these probably per material or something
	// Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
	// Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);

	//Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    
	std::promise<bool> fontLoadResult;
    
	// Load fonts before Gui //
    ThreadingManager::Get()->QueueTask(std::make_shared<QueuedTask>(std::bind<void>([](
                    std::promise<bool> &returnvalue, Graphics* us) -> void
        {
            us->Fonts = std::make_unique<Rendering::FontManager>();

            returnvalue.set_value(true);
            
        }, std::ref(fontLoadResult), this)));

    int displays = SDL_GetNumVideoDisplays();

    LOG_INFO("SDL: display count: " + Convert::ToString(displays));
    
    // Get display positions
    std::vector<SDL_Rect> displayBounds;
    
    for(int i = 0; i < displays; i++){
        
        displayBounds.push_back(SDL_Rect());
        
        SDL_GetDisplayBounds(i, &displayBounds.back());

        const char* nameptr = SDL_GetDisplayName(i);
        
        const auto name = nameptr ? std::string(nameptr) : std::string("unnamed");

        // Video modes //
        int videomodecount = SDL_GetNumDisplayModes(i);

        std::vector<std::string> videomodes;

        for(int a = 0; a < videomodecount; a++){


            SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };

            if(SDL_GetDisplayMode(i, a, &mode) == 0){
            
                videomodes.push_back(Convert::ToString(SDL_BITSPERPIXEL(mode.format)) +
                        " bpp " + Convert::ToString(mode.w) + "x" + Convert::ToString(mode.h)
                    + " at " + Convert::ToString(mode.refresh_rate) + "Hz");
            }
        }

        
        LOG_INFO("Display(" + Convert::ToString(i) + ", " + name + "): top left: (" +
            Convert::ToString(displayBounds.back().x) +
            ", " + Convert::ToString(displayBounds.back().y) + ") size: " +
            Convert::ToString(displayBounds.back().w) + "x" +
            Convert::ToString(displayBounds.back().h));

        // LOG_INFO("Supported modes(" + Convert::ToString(videomodes.size()) + "): ");
        // for(const auto& mode : videomodes){
            
        //     LOG_WRITE(" " + mode);
        // }
    }
    
    if(!fontLoadResult.get_future().get()){

        LOG_ERROR("Graphics: failed to load fonts");
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

    // TODO: check can we try to Tick here
    
	return true;
}
// ------------------------------------------- //


