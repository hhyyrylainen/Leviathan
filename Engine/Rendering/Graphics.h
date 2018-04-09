// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Application/AppDefine.h"

#include "OgreFrameListener.h"

namespace Leviathan {

class Graphics : Ogre::FrameListener {
    friend Window;

public:
    DLLEXPORT Graphics();
    DLLEXPORT ~Graphics();

    DLLEXPORT bool Init(AppDef* appdef);
    DLLEXPORT void Release();

    DLLEXPORT bool Frame();

    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    DLLEXPORT inline AppDef* GetDefinitionObject()
    {
        return AppDefinition;
    }
    DLLEXPORT inline Ogre::Root* GetOgreRoot()
    {
        return ORoot.get();
    }

    DLLEXPORT static Graphics* Get();

private:
    bool InitializeOgre(AppDef* appdef);

    //! \brief Load all the new required hlms stuff
    //! \warning This must be called after an Ogre window has been created
    //!
    //! This is called by the first created GraphicalInputEntity
    void _LoadOgreHLMS();

private:
    bool Initialized = false;

    AppDef* AppDefinition = nullptr;

    // OGRE //
    std::unique_ptr<Ogre::Root> ORoot;
    Ogre::Log* OLog = nullptr;

    // static //
    static Graphics* Staticaccess;
};
} // namespace Leviathan
