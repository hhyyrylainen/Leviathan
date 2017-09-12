#include "Engine.h"
#include "../PartialEngine.h"

#include "catch.hpp"

using namespace Leviathan;

class InvokeTestPartialEngine : public Test::PartialEngine<false>{
public:

    void RunInvokes(){

        {
            RecursiveLock lock(InvokeLock);
            CHECK(!InvokeQueue.empty());
        }

        ProcessInvokes();
    }
};

TEST_CASE("Invokes work", "[engine][threading]"){

    InvokeTestPartialEngine engine;

    SECTION("Basic"){

        bool invokeCalled = false;

        engine.Invoke([&](){

                invokeCalled = true;
            });
        
        engine.RunInvokes();

        CHECK(invokeCalled);
    }

    SECTION("Invoke causing invokes"){

        bool invokeCalled = false;

        engine.Invoke([&](){

                engine.Invoke([&](){

                        engine.Invoke([&](){

                                

                                invokeCalled = true;
                            });
                    });
            });
        
        engine.RunInvokes();

        CHECK(invokeCalled);        
    }
}

