#include "Events/EventHandler.h"

#include "catch.hpp"

using namespace Leviathan;
// using namespace Leviathan::Test;


TEST_CASE("Events are properly released by EventHandler", "[event]"){

    EventHandler handler;

    Event* testEvent = new Event(EVENT_TYPE_FRAME_END, new IntegerEventData(1));

    CHECK(testEvent->GetRefCount() == 1);

    testEvent->AddRef();
    
    CHECK(testEvent->GetRefCount() == 2);

    handler.CallEvent(testEvent);
    
    CHECK(testEvent->GetRefCount() == 1);

    testEvent->Release();
}



