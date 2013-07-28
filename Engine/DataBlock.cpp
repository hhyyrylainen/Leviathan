#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DATABLOCK
#include "DataBlock.h"
#endif
#include "ExceptionInvalidArguement.h"
#include "WstringIterator.h"
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

	// text to text check //
	VariableBlock tblocky(wstring(L"this is a test text that should be intact"));

	wstring checkvalue(L"");

	if(!tblocky.ConvertAndAssingToVariable<wstring>(checkvalue)){

		Failed = true;
	}
	// compare //
	if(checkvalue != L"this is a test text that should be intact"){

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
DLLEXPORT Leviathan::VariableBlock::VariableBlock(wstring &valuetoparse, vector<const NamedVariableBlock*>* predefined) throw(...){
	// the text should have all preceding and trailing spaces removed //
	if(valuetoparse.size() == 0){
		// can't be anything //
		throw ExceptionInvalidArguement(L"no data passed", valuetoparse.size(), __WFUNCTION__, L"valuetoparse", valuetoparse);
	}
	// try to figure out what type of block is required for this variable //

	// easy one first, quotes //
	if(valuetoparse[0] == L'"'){
		// it's a string //

		// use iterator to get data inside quotes //
		WstringIterator itr(&valuetoparse, false);

		unique_ptr<wstring> tempdata = itr.GetStringInQuotes(QUOTETYPE_DOUBLEQUOTES);

		// set data //
		// WstringBlock takes the pointer as it's own here //
		BlockData = new WstringBlock(tempdata.release());

		return;
	}
	// check does it contain non numeric characters //
	if(!Misc::WstringIsNumeric(valuetoparse)){

		// check does it match true/false //
		bool possiblevalue = false;

		if(Convert::IsWstringBool(valuetoparse, &possiblevalue)){

			BlockData = new BoolBlock(possiblevalue);

			return;
		}
		
		// check does some special value match it //
		if(predefined != NULL){
			// check do them match //
			for(size_t i = 0; i < predefined->size(); i++){

				if(predefined->at(i)->CompareName(valuetoparse)){
					// match found, copy value to here //
					BlockData = predefined->at(i)->GetBlockConst()->AllocateNewFromThis();
					return;
				}
			}
		}

		// create a string from the whole thing //
		BlockData = new WstringBlock(valuetoparse);

		return;
	}

	// try to figure out what kind of number it is //
	size_t decimalspot = valuetoparse.find_first_of(L'.');
	if(decimalspot != wstring::npos){
		// has decimal separator //

		// check does it need more decimal digits than float has // 

		if(valuetoparse.size()-1-decimalspot > FLT_DIG){
			// create a double //
			BlockData = new DoubleBlock(Convert::WstringTo<double>(valuetoparse));

		} else {

			// float should have space to hold all characters //
			BlockData = new FloatBlock(Convert::WstringTo<float>(valuetoparse));
		}

		return;
	}
	// should be plain old int //
	BlockData = new IntBlock(Convert::WstringTo<int>(valuetoparse));
}
