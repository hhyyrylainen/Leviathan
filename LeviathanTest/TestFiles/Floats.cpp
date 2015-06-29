#include "Common/Types.h"

#include "catch.hpp"

using namespace Leviathan;


TEST_CASE("Float4 access memory layout", "[types]"){

	// See if FloatX classes are properly castable //
	Float4 fl(1.f, 2.f, 3.f, 4.f);

	float* ptr = fl;
    
	CHECK(*ptr == fl.X);
    CHECK(fl[0] == fl.GetX());

    CHECK(*(ptr+1) == fl.Y);
    CHECK(fl[1] == fl.GetY());

	CHECK(*(ptr+2) == fl.Z);
    CHECK(fl[2] == fl.GetZ());

	CHECK(*(ptr+3) == fl.W);
    CHECK(fl[3] == fl.GetW());
}

