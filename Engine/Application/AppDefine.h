// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Networking/MasterServerInfo.h"

#include "Common/ThreadSafe.h"

#include <functional>

#ifdef _WIN32
#include "WindowsInclude.h"
#endif // _WIN32

namespace Leviathan{

class NamedVars;

struct WindowDataDetails{
    WindowDataDetails();
#ifdef _WIN32
    WindowDataDetails(const std::string &title, const int &width, const int &height,
        const bool &windowed, const bool &windowborder, HICON icon,
        LeviathanApplication* appvirtualptr);

    void ApplyIconToHandle(HWND hwnd) const;


    HICON Icon;
#else
    WindowDataDetails(const std::string &title, const int &width, const int &height,
        const bool &windowed, const bool &windowborder,
        LeviathanApplication* appvirtualptr);

#endif
    std::string Title;
    int Height;
    int Width;
    bool Windowed;
};


class AppDef{
    friend Engine;
public:
    DLLEXPORT AppDef(const bool &isdef = false);
    DLLEXPORT ~AppDef();


    DLLEXPORT NamedVars* GetValues();

    // named constructor functions //
#ifdef _WIN32
    DLLEXPORT AppDef& SetHInstance(HINSTANCE instance){

        HInstance = instance;
        return *this;
    }

    DLLEXPORT HINSTANCE GetHInstance(){

        return HInstance;
    }
#else
    // \todo linux equivalent
#endif
    DLLEXPORT AppDef& SetWindowDetails(const WindowDataDetails &det){

        WDetails = det;
        return *this;
    }

    DLLEXPORT AppDef& SetMasterServerParameters(const MasterServerInformation &info){

        MasterServerInfo = info;
        return *this;
    }
    // Sets the version information of the application, leviathan version is set automatically //
    DLLEXPORT AppDef& SetApplicationIdentification(const std::string &userreadable,
        const std::string &gamename, const std::string &gameversion);

    DLLEXPORT WindowDataDetails& GetWindowDetails(){

        return WDetails;
    }
    DLLEXPORT MasterServerInformation& GetMasterServerInfo(){

        return MasterServerInfo;
    }
    DLLEXPORT inline static AppDef* GetDefault(){
        return Defaultconf;
    }
    DLLEXPORT bool GetVSync();

    DLLEXPORT static AppDef* GenerateAppdefine(const std::string &engineconfigfile,
        const std::string &gameconfig, const std::string &keyconfig, 
        std::function<void (Lock &guard, GameConfiguration* configobj)> configchecker,
        std::function<void (Lock &guard, KeyConfiguration* keysobject)> keychecker);
        
#ifdef _WIN32
    DLLEXPORT void StoreWindowDetails(const std::string &title, const bool &windowborder,
        HICON icon, LeviathanApplication* appvirtualptr);
#else
    DLLEXPORT void StoreWindowDetails(const std::string &title, const bool &windowborder,
        LeviathanApplication* appvirtualptr);
#endif


    DLLEXPORT void GetGameIdentificationData(std::string &userreadable, std::string &gamename,
        std::string &gameversion);

protected:

    std::unique_ptr<NamedVars> ConfigurationValues;
#ifdef _WIN32
    HINSTANCE HInstance;
#else
    int HInstance;
#endif
    MasterServerInformation MasterServerInfo;

    std::shared_ptr<NetworkInterface> _NetworkInterface;

    // details used to create a window //
    WindowDataDetails WDetails;

    // Game variables //
    GameConfiguration* _GameConfiguration = nullptr;
    KeyConfiguration* _KeyConfiguration = nullptr;

    Logger* Mainlog = nullptr;


    //! Controls whether the destructor deletes Mainlog
    //! \note Used to not delete loggers that weren't created by this instance
    bool DeleteLog = false;

    std::string LeviathanVersion;
    std::string GameVersion;
    std::string Game;
    std::string UserReadableGame;

    // ------------------------------------ //
    static AppDef* Defaultconf;
};
}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::AppDef;
using Leviathan::MasterServerInformation;
#endif


