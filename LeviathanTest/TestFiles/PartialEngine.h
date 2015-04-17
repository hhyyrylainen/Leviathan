/**
   \file Utility to create many partially initialized Engine objects
*/
#include "Application/Application.h"
#include "Engine.h"
#include "Events/EventHandler.h"
#include "Newton/NewtonManager.h"
#include "Newton/PhysicsMaterialManager.h"

#include "catch.hpp"

using namespace Leviathan;

//! \brief Implementation for application for tests
class PartialApplication : public LeviathanApplication{
public:

};

//! \brief Partial implementation of Leviathan::Engine for tests
template<bool UseActualInit, NETWORKED_TYPE TestWithType>
class PartialEngine : public Engine{
public:

    PartialEngine() : Log(L"Test/TestLog.txt"), Def(), App(), Engine(&App){

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

            _NewtonManager = new NewtonManager();
            PhysMaterials = new PhysicsMaterialManager(_NewtonManager);
        }
    }

    ~PartialEngine(){

        Log.Save();
        
        SAFE_DELETE(PhysMaterials);
        SAFE_DELETE(_NewtonManager);
        if(UseActualInit){

            SAFE_RELEASEDEL(_NetworkHandler);
        } else {
            // This wasn't initialized //
            SAFE_DELETE(_NetworkHandler);
        }
        SAFE_RELEASEDEL(MainEvents);
    }
    

    PartialApplication App;
    Logger Log;
    AppDef Def;
};
