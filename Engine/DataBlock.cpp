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

	if(vblock.IsConversionAllowedNonPtr<float>() == false){
		// should be valid conversion //

		QUICK_ERROR_MESSAGE;
		Failed = true;
	} else {
		// this needs to be skipped if not allowed //

		float fvaluegot = vblock;
		if(fvaluegot != TEST_IVALUE_INDBLOCKS){

			QUICK_ERROR_MESSAGE;
			Failed = true;
		}
	}

	FloatBlock fblock(TEST_IVALUE_INDBLOCKS);

	gotvalue = fblock;

	if(gotvalue != TEST_IVALUE_INDBLOCKS){

		QUICK_ERROR_MESSAGE;
		Failed = true;
	}

	wstring texty = iblock;

	if(texty != Convert::ToWstring(TEST_IVALUE_INDBLOCKS)){

		QUICK_ERROR_MESSAGE;
		Failed = true;
	}

	// check fail state //
	if(Failed){

		return true;
	}

	// stress testing //
	for(int i = 0; i < tests; i++){

		unique_ptr<VariableBlock> tempnewblock(new VariableBlock(new IntBlock(2524646)));
		int values = *tempnewblock;

		*tempnewblock = new FloatBlock(2546);

		values = values*((int)*tempnewblock);
	}
	return false;
}


// ------------------------------------ //


