#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DATABLOCK
#include "DataBlock.h"
#endif
#include "Exceptions.h"
#include "Iterators/StringIterator.h"
#include "Script/ScriptExecutor.h"
#include "../StringOperations.h"
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

	// void block testings //

	Float4* floaty = new Float4(1, 564, 73784, 124);

	VoidPtrBlock vblocky(floaty);

	Float4* returnptr = (Float4*)(void*)(vblocky);

	if(returnptr != floaty || *returnptr != *floaty){

		QUICK_ERROR_MESSAGE;
		Failed = true;
	}

	SAFE_DELETE(floaty);


	// check fail state //
	if(Failed){

		return true;
	}
	int values;
	// stress testing //
	for(int i = 0; i < tests; i++){

		unique_ptr<VariableBlock> tempnewblock(new VariableBlock(new IntBlock(2524646)));
		values = *tempnewblock;

		*tempnewblock = new FloatBlock(2546);

		values = values*((int)*tempnewblock);
	}
	return false;
}

//
// Template specifications should be fine being in here without causing any issues
// should also improve compilation speed and reduce the mess in the header and
// allow includes that are needed and couldn't be in the header
//
namespace Leviathan{
// templates for getting AngelScript type id from template //
#define TYPEIDGETTEMPLATEINSTANTIATION(TypeForTemplate, StringToUse) template<> \
    struct TypeToAngelScriptIDConverter<TypeForTemplate>{static inline int \
        GetTypeIDFromTemplated(){ return \
                ScriptExecutor::Get()->GetAngelScriptTypeID(L"int"); }};


TYPEIDGETTEMPLATEINSTANTIATION(int, L"int");
TYPEIDGETTEMPLATEINSTANTIATION(bool, L"bool");
TYPEIDGETTEMPLATEINSTANTIATION(float, L"float");
TYPEIDGETTEMPLATEINSTANTIATION(char, L"char");
TYPEIDGETTEMPLATEINSTANTIATION(string, L"string");

}

// ------------------------------------ //
DLLEXPORT Leviathan::VariableBlock::VariableBlock(const wstring &valuetoparse,
    map<wstring, shared_ptr<VariableBlock>>* predefined) THROWS
{
	// the text should have all preceding and trailing spaces removed //
	if(valuetoparse.size() == 0){
		// can't be anything //
		throw InvalidArgument("no data passed");
	}
	// try to figure out what type of block is required for this variable //

	// easy one first, quotes //
	if(valuetoparse[0] == L'"'){
		// it's a string //

		// use iterator to get data inside quotes //
		StringIterator itr(valuetoparse);

		unique_ptr<wstring> tempdata = itr.GetStringInQuotes<wstring>(QUOTETYPE_DOUBLEQUOTES);

		// set data //
		// WstringBlock takes the pointer as it's own here //
		BlockData = new WstringBlock(tempdata ? tempdata.release(): new wstring());

		return;
	}
    
	// check does it contain non numeric characters //
	if(!StringOperations::IsStringNumeric<wstring, wchar_t>(valuetoparse)){

		// check does it match true/false //
		bool possiblevalue = false;

		if(Convert::IsWstringBool(valuetoparse, &possiblevalue)){

			BlockData = new BoolBlock(possiblevalue);

			return;
		}

		// check does some special value match it //
		if(predefined != NULL){
			// check do them match //

			auto ivaliterator = predefined->find(valuetoparse);

			if(ivaliterator != predefined->end()){
				// found! //

				BlockData = ivaliterator->second->GetBlockConst()->AllocateNewFromThis();
				return;
			}
		}

		// create a string from the whole thing //
		BlockData = new WstringBlock(valuetoparse);

		return;
	}

	// Try to figure out what kind of a number it is //
	size_t decimalspot = valuetoparse.find_first_of(L'.');
	if(decimalspot != wstring::npos){
		// has decimal separator //

		// check does it need more decimal digits than a float has //

		if(valuetoparse.size()-1-decimalspot > FLT_DIG){
			// create a double //
			BlockData = new DoubleBlock(Convert::WstringTo<double>(valuetoparse));

		} else {

			// float should have space to hold all characters //
			BlockData = new FloatBlock(Convert::WstringTo<float>(valuetoparse));
		}

		return;
	}

	// Should be a plain old int //
	BlockData = new IntBlock(Convert::WstringTo<int>(valuetoparse));
}


// ------------------ ScriptSafeVariableBlock ------------------ //
Leviathan::ScriptSafeVariableBlock::ScriptSafeVariableBlock(VariableBlock* copyfrom,
    const wstring &name) :
    NamedVariableBlock(copyfrom->GetBlock()->AllocateNewFromThis(), name)
{
		// we need to copy all settings from the block //
		switch(copyfrom->GetBlock()->Type){
		case DATABLOCK_TYPE_INT: ASTypeID = TypeToAngelScriptIDConverter<int>::GetTypeIDFromTemplated(); break;
		case DATABLOCK_TYPE_FLOAT: ASTypeID = TypeToAngelScriptIDConverter<float>::GetTypeIDFromTemplated(); break;
		case DATABLOCK_TYPE_BOOL: ASTypeID = TypeToAngelScriptIDConverter<bool>::GetTypeIDFromTemplated(); break;
		case DATABLOCK_TYPE_WSTRING:
			{
				// we'll use automatic conversion here //
				unique_ptr<DataBlockAll> tmp(new StringBlock(ConvertAndReturnVariable<string>()));

				SAFE_DELETE(BlockData);
				BlockData = tmp.release();

				ASTypeID = TypeToAngelScriptIDConverter<string>::GetTypeIDFromTemplated(); break;
			}
		break;
		case DATABLOCK_TYPE_STRING: ASTypeID = TypeToAngelScriptIDConverter<string>::GetTypeIDFromTemplated(); break;
		case DATABLOCK_TYPE_CHAR: ASTypeID = TypeToAngelScriptIDConverter<char>::GetTypeIDFromTemplated(); break;
		case DATABLOCK_TYPE_DOUBLE: ASTypeID = TypeToAngelScriptIDConverter<double>::GetTypeIDFromTemplated(); break;

		default:
			throw InvalidArgument("cannot convert non-named, generic type block to script safe block");
		}
}

// ------------------ Loading/saving from/to packets ------------------ //
#define DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(BlockTypeName, VarTypeName, TmpTypeName) \
template<> DLLEXPORT void BlockTypeName::AddDataToPacket(sf::Packet &packet){ \
	packet << *Value; \
} \
template<> DLLEXPORT BlockTypeName::DataBlock(sf::Packet &packet){ \
	Type = DataBlockNameResolver<VarTypeName>::TVal; \
	TmpTypeName tmpval; \
	if(!(packet >> tmpval)){ \
		throw InvalidArgument("invalid packet format"); \
	} \
	Value = new VarTypeName(tmpval); \
}



// ------------------ Loading/saving from/to packets ------------------ //
namespace Leviathan{

	DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(IntBlock, int, int);
	DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(FloatBlock, float, float);
	DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(BoolBlock, bool, bool);
	DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(WstringBlock, wstring, wstring);
	DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(StringBlock, string, string);
	DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(DoubleBlock, double, double);
	DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(CharBlock, char, sf::Int8);



	// Fill in the gaps in the templates with these defaults //
	template<class DBlockT> DLLEXPORT void Leviathan::DataBlock<DBlockT>::AddDataToPacket(sf::Packet&)
	{
		// The default one cannot do anything, only the specialized functions can try to do something //
		throw ExceptionBase(L"this type doesn't support saving to a packet", 0, __WFUNCTION__);
	}
	template<class DBlockT> DLLEXPORT Leviathan::DataBlock<DBlockT>::DataBlock(sf::Packet&)
	{
		// The default one cannot do anything, only the specialized functions can try to do something //
		throw ExceptionBase(L"this type doesn't support loading from a packet", 0, __WFUNCTION__);
	}

	// ------------------ VariableBlock ------------------ //
	DLLEXPORT void VariableBlock::AddDataToPacket(sf::Packet& packet) const
	{
		// Set the type //
		if(BlockData != NULL){
			packet << BlockData->Type;
		} else{
			packet << 0;
			return;
		}

		// Set the data //
		if(BlockData->Type == DATABLOCK_TYPE_INT)
			return TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(BlockData)->AddDataToPacket(packet);
		else if(BlockData->Type == DATABLOCK_TYPE_FLOAT)
			return TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(BlockData)->AddDataToPacket(packet);
		else if(BlockData->Type == DATABLOCK_TYPE_BOOL)
			return TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(BlockData)->AddDataToPacket(packet);
		else if(BlockData->Type == DATABLOCK_TYPE_WSTRING)
			return TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(BlockData)->AddDataToPacket(packet);
		else if(BlockData->Type == DATABLOCK_TYPE_STRING)
			return TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(BlockData)->AddDataToPacket(packet);
		else if(BlockData->Type == DATABLOCK_TYPE_CHAR)
			return TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(BlockData)->AddDataToPacket(packet);
		else if(BlockData->Type == DATABLOCK_TYPE_DOUBLE)
			return TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(BlockData)->AddDataToPacket(packet);

		// type that shouldn't be used is used //
		throw InvalidType("unallowed datatype in datablock for writing to packet");
	}
	
	DLLEXPORT VariableBlock::VariableBlock(sf::Packet& packet)
	{

		// Get the type //
		short type;
		packet >> type;

		// Load the actual data based on the type //
		switch(type){
		case 0:
		{
			// No data //
			BlockData = NULL;
			return;
		}
		case DATABLOCK_TYPE_INT:
		{
			BlockData = new IntBlock(packet);
			return;
		}
		case DATABLOCK_TYPE_FLOAT:
		{
			BlockData = new FloatBlock(packet);
			return;
		}
		case DATABLOCK_TYPE_BOOL:
		{
			BlockData = new BoolBlock(packet);
			return;
		}
		case DATABLOCK_TYPE_WSTRING:
		{
			BlockData = new WstringBlock(packet);
			return;
		}
		case DATABLOCK_TYPE_STRING:
		{
			BlockData = new StringBlock(packet);
			return;
		}
		case DATABLOCK_TYPE_CHAR:
		{
			BlockData = new CharBlock(packet);
			return;
		}
		case DATABLOCK_TYPE_DOUBLE:
		{
			BlockData = new DoubleBlock(packet);
			return;
		}
		}

		// Invalid packet //
		throw InvalidArgument("invalid packet format");
	}

}
