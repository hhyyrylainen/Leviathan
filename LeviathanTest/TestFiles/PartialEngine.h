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

//! This is required to reduce the times newton is initialized/released as that seems to be a
//! major cause of deadlocking
class NewtonHolder{
public:

    static std::unique_ptr<NewtonManager> StaticNewtonManager;
    static std::unique_ptr<PhysicsMaterialManager> StaticPhysicsMaterialManager;
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

            if(!NewtonHolder::StaticNewtonManager){

                NewtonHolder::StaticNewtonManager = std::move(std::unique_ptr<NewtonManager>(
                        new NewtonManager));
                NewtonHolder::StaticPhysicsMaterialManager = std::move(
                    std::unique_ptr<PhysicsMaterialManager>(new
                        PhysicsMaterialManager(NewtonHolder::StaticNewtonManager.get())));
            }
            
            _NewtonManager = NewtonHolder::StaticNewtonManager.get();
            PhysMaterials = NewtonHolder::StaticPhysicsMaterialManager.get();

            REQUIRE(PhysicsMaterialManager::Get() != nullptr);
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

    void AdjustTickClock(int mstoset){

        _AdjustTickClock(mstoset, true);
    }
    

    PartialApplication App;
    Logger Log;
    AppDef Def;


};
