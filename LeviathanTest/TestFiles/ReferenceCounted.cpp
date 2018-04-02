#include "Common/ReferenceCounted.h"

#include "catch.hpp"

using namespace Leviathan;

// This is for making sure the constructors and destructors of OurCoolReferenceCountedThing run
int OurCoolThingRunStatus = 0;

class OurCoolReferenceCountedThing : public ReferenceCounted{
public:

    REFERENCE_COUNTED_PTR_TYPE(OurCoolReferenceCountedThing);

    int Magic = -1;

protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    
    OurCoolReferenceCountedThing(int magic) : Magic(magic){

        OurCoolThingRunStatus = 1;
    }

    ~OurCoolReferenceCountedThing(){

        OurCoolThingRunStatus = 0;
    }
};


TEST_CASE("ReferenceCounted::MakeShared works", "[engine]"){
    
    // This may *not* compile: OurCoolReferenceCountedThing thing(1);

    // Simple test
    OurCoolReferenceCountedThing::pointer ptr;

    CHECK(!ptr);
    CHECK(OurCoolThingRunStatus == 0);

    constexpr auto magic = 41;

    ptr = OurCoolReferenceCountedThing::MakeShared<OurCoolReferenceCountedThing>(magic);

    CHECK(ptr);
    CHECK(OurCoolThingRunStatus == 1);

    CHECK(ptr->Magic == magic);

    ptr.reset();
    
    CHECK(!ptr);
    CHECK(OurCoolThingRunStatus == 0);    
}

