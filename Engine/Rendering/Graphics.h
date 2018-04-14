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

#ifdef __linux
    //! \brief Returns true if our X11 error handler has been called. Remember to check this
    //! after every X11 call
    //!
    //! The value is reset to false after this call
    //! \note This is not thread safe. X11 is also not thread safe so only call on the main
    //! thread
    DLLEXPORT static bool HasX11ErrorOccured();
#endif

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
