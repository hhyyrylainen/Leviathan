#include "Common/DataStoring/DataBlock.h"
#include "Utility/Convert.h"
#include "Common/Types.h"

#include <memory>

#include <catch.hpp>

using namespace Leviathan;
using namespace std;

#define TEST_IVALUE_INDBLOCKS 254676

TEST_CASE("DataBlock value casts", "[datablock][script][networking]"){


    IntBlock iblock(TEST_IVALUE_INDBLOCKS);

	int gotvalue = iblock;

    CHECK(gotvalue == TEST_IVALUE_INDBLOCKS);

	VariableBlock vblock(new IntBlock(TEST_IVALUE_INDBLOCKS));

	gotvalue = vblock;

	CHECK(gotvalue == TEST_IVALUE_INDBLOCKS);


	REQUIRE(vblock.IsConversionAllowedNonPtr<float>() == true);

    // this needs to be skipped if not allowed //
    float fvaluegot = vblock;
    CHECK(fvaluegot == TEST_IVALUE_INDBLOCKS);

	FloatBlock fblock(TEST_IVALUE_INDBLOCKS);

	gotvalue = fblock;

    CHECK(gotvalue == TEST_IVALUE_INDBLOCKS);

	wstring texty = iblock;

	CHECK(texty == Convert::ToWstring(TEST_IVALUE_INDBLOCKS));

	// text to text check //
	VariableBlock tblocky(wstring(L"this is a test text that should be intact"));

	wstring checkvalue;

	REQUIRE(tblocky.ConvertAndAssingToVariable<wstring>(checkvalue));

	CHECK(checkvalue == L"this is a test text that should be intact");

	// void block testings //

	unique_ptr<Float4> floaty = make_unique<Float4>(1.f, 564.f, 73784.f, 124.f);

	VoidPtrBlock vblocky(floaty.get());

	Float4* returnptr = (Float4*)(void*)(vblocky);

	CHECK(returnptr == floaty.get());
    CHECK(*returnptr == *floaty);
}

TEST_CASE("DataBlock void ptr cast", "[datablock]"){

    void* testptr = reinterpret_cast<void*>(0x42);

    auto block = std::make_shared<NamedVariableBlock>(new VoidPtrBlock(testptr), "Block1");

    void* returned = static_cast<void*>(*block);

    CHECK(returned == testptr);
}





