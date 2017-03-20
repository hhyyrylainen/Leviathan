#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "GraphicalInputEntity.h"
#include "Application/AppDefine.h"
#include "OgreFrameListener.h"

namespace Leviathan{
// forward declarations to avoid having tons of headers here that aren't necessary //
namespace Rendering{
        
class ShaderManager;
class FontManager;
}

class Graphics : Ogre::FrameListener{
public:
    DLLEXPORT Graphics();
    DLLEXPORT ~Graphics();

    DLLEXPORT bool Init(AppDef* appdef);
    DLLEXPORT void Release();

    DLLEXPORT bool Frame();

    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    DLLEXPORT inline Rendering::FontManager* GetFontManager(){
        return Fonts.get();
    }
    DLLEXPORT inline AppDef* GetDefinitionObject(){
        return AppDefinition;
    }
    DLLEXPORT inline Ogre::Root* GetOgreRoot(){
        return ORoot.get();
    }

    DLLEXPORT static Graphics* Get();
private:

    bool InitializeOgre(AppDef* appdef);
    // ------------------------ //
    bool Initialized = false;

    AppDef* AppDefinition = nullptr;

    // OGRE //
    std::unique_ptr<Ogre::Root> ORoot;
    Ogre::Log* OLog = nullptr;
    std::unique_ptr<Rendering::FontManager> Fonts;

    // static //
    static Graphics* Staticaccess;
};
}

