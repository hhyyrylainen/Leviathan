/**
   \file Utility to create many partially initialized Engine objects
*/
#include "Application/Application.h"
#include "Engine.h"
#include "Events/EventHandler.h"
#include "TimeIncludes.h"
#include "Networking/NetworkClientInterface.h"

#include "catch.hpp"

using namespace Leviathan;

//! \brief Implementation for application for tests
class PartialApplication : public LeviathanApplication{
public:

};

class PartialClient : public NetworkClientInterface{
public:
    void _OnStartApplicationConnect() override{
        
    }
};

//! \brief Partial implementation of Leviathan::Engine for tests
template<bool UseActualInit, NETWORKED_TYPE TestWithType>
class PartialEngine : public Engine{
public:

    PartialEngine() : Log("Test/TestLog.txt"), Def(), App(), Engine(&App){

        // Configure for test use //
        NoGui = true;
        NoLeap = true;
        NoSTDInput = true;
        
        // Setup some core values //
        if(UseActualInit){

            bool succeeded = Init(&Def, TestWithType);

            REQUIRE(succeeded);
            
        } else {

            Define = &Def;

            MainEvents = new EventHandler();

            _NetworkHandler = new NetworkHandler(TestWithType, NULL);

            IDDefaultInstance = new IDFactory();
        }
    }

    ~PartialEngine(){

        Log.Save();
        
        if(UseActualInit){

            Release();
            return;
        }

        // This wasn't initialized //
        SAFE_DELETE(_NetworkHandler);
            
        SAFE_RELEASEDEL(MainEvents);

        SAFE_DELETE(IDDefaultInstance);
    }

    void ResetClock(int mstoset){

        LastTickTime = Time::GetTimeMs64()-mstoset;
    }

    PartialApplication App;
    Logger Log;
    AppDef Def;

    PartialClient DummyClient;
};
