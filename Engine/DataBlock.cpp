#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DATABLOCK
#include "DataBlock.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

#define TEST_IVALUE_INDBLOCKS		254676

DLLEXPORT bool Leviathan::DataBlockTestVerifier(const int &tests){

	bool Failed = false;

	IntBlock iblock(TEST_IVALUE_INDBLOCKS);

	int gotvalue = iblock;

	if(gotvalue != TEST_IVALUE_INDBLOCKS){

		QUICK_ERROR_MESSAGE;
		Failed = true;
	}

	VariableBlock vblock(new IntBlock(TEST_IVALUE_INDBLOCKS));

	gotvalue = vblock;

	if(gotvalue != TEST_IVALUE_INDBLOCKS){

		QUICK_ERROR_MESSAGE;
		Failed = true;
	}

	// check fail state //
	if(Failed){

		return true;
	}

	// stress testing //
	for(int i = 0; i < tests; i++){


	}
	return false;
}


// ------------------------------------ //


