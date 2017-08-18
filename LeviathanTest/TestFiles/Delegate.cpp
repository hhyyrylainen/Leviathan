#include "Events/DelegateSlot.h"

#include "catch.hpp"

using namespace Leviathan;

TEST_CASE("Lambda delegates work", "[delegate]"){

    Delegate slot;

    bool lambda1Called = false;
    bool lambda2Called = false;

    slot.Register(LambdaDelegateSlot::MakeShared(new LambdaDelegateSlot(
                [&](const NamedVars::pointer&) -> void {

                    lambda1Called = true;
                    
                }))); 

    slot.Register(LambdaDelegateSlot::MakeShared(new LambdaDelegateSlot(
                [&](const NamedVars::pointer&) -> void {

                    lambda2Called = true;
                    
                }))); 
    
    CHECK(!lambda1Called);
    CHECK(!lambda2Called);

    slot.Call(NamedVars::MakeShared(new NamedVars()));

    CHECK(lambda1Called);
    CHECK(lambda2Called);    
}

