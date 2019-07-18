/**
   \file Utility to create many partially initialized Engine objects
*/
#pragma once

#include "Application/Application.h"
#include "Engine.h"
#include "Events/EventHandler.h"
#include "Networking/NetworkClientInterface.h"
#include "Networking/NetworkHandler.h"
#include "TimeIncludes.h"

#include "FileSystem.h"

#include "Utility/Random.h"

#include <string>

#include "catch/catch.hpp"

#include "DummyLog.h"

namespace Leviathan { namespace Test {

//! \brief Implementation for application for tests
class PartialApplication : public LeviathanApplication {
public:
    NETWORKED_TYPE GetProgramNetType() const override
    {

        // Don't want to mimic either client or server to make testing easier
        return NETWORKED_TYPE::Master;
    }

    //! Not used
    NetworkInterface* _GetApplicationPacketHandler() override
    {

        return nullptr;
    }

    void _ShutdownApplicationPacketHandler() override {}
};

class PartialClient : public NetworkClientInterface {
public:
    void _OnProperlyConnected() override {}
};


//! \version Now puts all output to the catch framework which only
//! prints it if a test fails. That doesn't actually work
//! \todo Fix all the output not being visible
//! \todo Merge this with DummyLogger if that makes sense
class TestLogger : public Logger {
public:
    TestLogger(const std::string& file) : Logger(file) {}
    TestLogger() : Logger("Test/TestLog.txt") {}

    void Info(const std::string& data) override
    {
        INFO(data);
    }

    void Error(const std::string& data) override
    {
        Logger::Error(data);
        FAIL(data);
    }

    void Warning(const std::string& data) override
    {
        Logger::Warning(data);

        if(!IgnoreWarnings)
            FAIL(data);
    }

    void Fatal(const std::string& text) override
    {
        FAIL(text);
        REQUIRE(false);
    }

    void Write(const std::string& text) override
    {
        Logger::Write(text);

        // Detect AngelScript warnings and errors
        if(text.find("[SCRIPT] [WARNING]") == 0) {

            if(!IgnoreWarnings)
                FAIL(text);
        } else if(text.find("[SCRIPT] [ERROR]") == 0) {
            FAIL(text);
        }
    }

    bool IgnoreWarnings = false;
};

//! Version of TestLogger that needs an error to be reported
class TestLogRequireError : public Logger {
public:
    TestLogRequireError(const std::string& file) : Logger(file) {}
    TestLogRequireError() : Logger("Test/TestLog.txt") {}

    ~TestLogRequireError()
    {

        CHECK(ErrorOccured);
    }

    void Write(const std::string& data) override
    {

        INFO(data);
    }

    void Info(const std::string& data) override
    {

        INFO(data);
    }

    void Error(const std::string& data) override
    {

        ErrorOccured = true;
    }

    void Warning(const std::string& data) override
    {

        if(WarningsCountAsErrors) {

            ErrorOccured = true;

        } else {

            Logger::Warning(data);

            INFO(data);
        }
    }

    void Fatal(const std::string& text) override
    {

        FAIL(text);
        REQUIRE(false);
    }

    bool ErrorOccured = false;
    bool WarningsCountAsErrors = false;
};

//! Version of TestLogger that requires specific messages to be printed
class TestLogMatchMessagesRegex : public Logger, public ReporterMatchMessagesRegex {
public:
    TestLogMatchMessagesRegex() : Logger("Test/TestLog.txt")
    {

        CheckWrite = true;
    }

    void Write(const std::string& data) override
    {
        ReporterMatchMessagesRegex::Write(data);
    }

    void Info(const std::string& data) override
    {
        ReporterMatchMessagesRegex::Info(data);
    }

    void Error(const std::string& data) override
    {
        ReporterMatchMessagesRegex::Error(data);
    }

    void Warning(const std::string& data) override
    {
        ReporterMatchMessagesRegex::Warning(data);
    }

    void Fatal(const std::string& text) override
    {
        FAIL(text);
        REQUIRE(false);
    }
};


//! \brief Partial implementation of Leviathan::Engine for tests
template<bool UseActualInit>
class PartialEngine : public Engine {
public:
    PartialEngine(NetworkHandler* handler = nullptr) : Engine(&App), Log("Test/TestLog.txt")
    {
        // Configure for test use //
        NoGui = true;
        NoLeap = true;
        NoSTDInput = true;

        // Setup some core values //
        if(UseActualInit) {

            REQUIRE(handler);
            bool succeeded = Init(&Def, handler->GetNetworkType(), nullptr);

            REQUIRE(succeeded);

        } else {

            Define = &Def;

            MainEvents = new EventHandler();

            _NetworkHandler = handler;

            IDDefaultInstance = new IDFactory();
        }

        instance = this;
    }

    ~PartialEngine()
    {
        Log.Save();

        if(UseActualInit) {

            Release();
            return;
        }

        // This wasn't initialized //
        SAFE_DELETE(_NetworkHandler);

        SAFE_RELEASEDEL(MainEvents);

        SAFE_DELETE(IDDefaultInstance);

        SAFE_DELETE(MainRandom);
    }

    //! Creates random support
    void InitRandomForTest()
    {
        // Always same number to have reproducible tests
        MainRandom = new Random(42);
        MainRandom->SetAsMain();
    }

    void ResetClock(int mstoset)
    {
        LastTickTime = Time::GetTimeMs64() - mstoset;
    }

    PartialApplication App;
    TestLogger Log;
    AppDef Def;
};

// TODO: bsf doesn't have a proper window-less mode yet and it takes a while to startup...
// //! Partial Engine with window-less Ogre for GUI and other tests that need Ogre components
// //! \todo Do something with this now that Ogre is gone
// class PartialEngineWithOgre : public PartialEngine<false> {
// public:
//     PartialEngineWithOgre(NetworkHandler* handler = nullptr, SoundDevice* sound = nullptr) :
//         PartialEngine(handler)
//     {
//         // TODO: allow the Graphics object to be used here
//         // Suppress log
//         //         Ogre::Log* ogreLog =
//         //             OgreLogManager.createLog("Test/TestOgreLog.txt", true, false, false);

//         //         REQUIRE(ogreLog == OgreLogManager.getDefaultLog());

//         //         root = new Ogre::Root("", "", "");

//         //         Ogre::String renderSystemName = "RenderSystem_GL3Plus";

//         // #ifdef _DEBUG
//         //         renderSystemName.append("_d");
//         // #endif // _DEBUG

//         // #ifndef _WIN32
//         //         // On platforms where rpath works plugins are in the lib subdirectory
//         //         renderSystemName = "lib/" + renderSystemName;
//         // #endif

//         //         root->loadPlugin(renderSystemName);
//         //         const auto& renderers = root->getAvailableRenderers();
//         //         REQUIRE(renderers.size() > 0);
//         //         REQUIRE(renderers[0]);
//         //         root->setRenderSystem(renderers[0]);
//         //         root->initialise(false, "", "");

//         MainFileHandler = new FileSystem();

//         REQUIRE(MainFileHandler->Init(&Log));

//         // Register resources to Ogre //
//         // MainFileHandler->RegisterOGREResourceGroups(true);

//         Sound = sound;
//     }

//     ~PartialEngineWithOgre()
//     {
//         SAFE_DELETE(MainFileHandler);
//         // SAFE_DELETE(root);
//     }

//     // Ogre::LogManager OgreLogManager;
//     // Ogre::Root* root;
// };

}} // namespace Leviathan::Test
