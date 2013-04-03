#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DATABLOCK
#include "DataBlock.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DataBlock::DataBlock(){
	Type = DATABLOCK_TYPE_ERROR;
}
DataBlock::DataBlock(int type){
	Type = type;
}
DataBlock* DataBlock::CopyConstructor(DataBlock* arg){
	DataBlock* tmp = NULL;

	// get type //
	if(typeid(arg) == typeid(IntBlock)){
		tmp = new IntBlock((IntBlock*)arg);
	}
	if(typeid(arg) == typeid(FloatBlock)){
		tmp = new FloatBlock((FloatBlock*)arg);
	}
	if(typeid(arg) == typeid(BoolBlock)){
		tmp = new BoolBlock((BoolBlock*)arg);
	}
	if(typeid(arg) == typeid(WstringBlock)){
		tmp = new WstringBlock((WstringBlock*)arg);
	}
	if(typeid(arg) == typeid(VoidBlock)){
		tmp = new VoidBlock((VoidBlock*)arg);
	}
	tmp->Type = arg->Type;
	return tmp;
}
DataBlock::~DataBlock(){
	Type = DATABLOCK_TYPE_ERROR;
}
// ------------------------------------ //
DataBlock DataBlock::operator=(const DataBlock& arg){
	return (DataBlock(arg.Type));
}

// ------------------------------------ //
DataBlock::operator int(){
	if(Type == DATABLOCK_TYPE_ERROR){

		Logger::Get()->Error(L"DataBlock: operator int converting block of type error", Type, true);
		return 80000800;
	}
	if(Type == DATABLOCK_TYPE_INT){
		return (int)*((IntBlock*)this);
	}
	switch(Type){
	//case DATABLOCK_TYPE_INT:
		case DATABLOCK_TYPE_FLOAT:
			{
				return (int)(float)*((FloatBlock*)this);
			}
		break;
		case DATABLOCK_TYPE_BOOL:
			{
				return (int)(bool)*((BoolBlock*)this);
			}
		break;
		case DATABLOCK_TYPE_WSTRING:
			{
				return Convert::WstringToInt(*(wstring*)*((WstringBlock*)this));
			}
		break;
		//case DATABLOCK_TYPE_VOIDPTR:
		default:
			{
				Logger::Get()->Error(L"DataBlock: No conversion exists between int and ", Type, true);
				return 80000801;
			}
		break;
	}
}
DataBlock::operator int*(){
	if(Type == DATABLOCK_TYPE_ERROR){

		Logger::Get()->Error(L"DataBlock: operator int converting block of type error", Type, true);
		return NULL;
	}
	if(Type == DATABLOCK_TYPE_INT){
		return (int*)*((IntBlock*)this);
	}
	switch(Type){
	//case DATABLOCK_TYPE_INT:
		case DATABLOCK_TYPE_FLOAT:
			{
				return (int*)(float*)*((FloatBlock*)this);
			}
		break;
		case DATABLOCK_TYPE_BOOL:
			{
				return (int*)(bool*)*((BoolBlock*)this);
			}
		break;
		case DATABLOCK_TYPE_WSTRING:
			{
				Logger::Get()->Error(L"Calling conversion on wstring to int: LEAKED MEMORY", 80000800, true);
				return new int(Convert::WstringToInt(*(wstring*)*((WstringBlock*)this)));
			}
		break;
		//case DATABLOCK_TYPE_VOIDPTR:
		default:
			{
				Logger::Get()->Error(L"DataBlock: No conversion exists between int and ", Type, true);
				return NULL;
			}
		break;
	}
}

DataBlock::operator float(){
	if(Type == DATABLOCK_TYPE_ERROR){

		Logger::Get()->Error(L"DataBlock: operator float converting block of type error", Type, true);
		return 80000800;
	}
	if(Type == DATABLOCK_TYPE_FLOAT){
		return (float)*((FloatBlock*)this);
	}
	switch(Type){
	//case DATABLOCK_TYPE_INT:
		case DATABLOCK_TYPE_INT:
			{
				return (float)(int)*((IntBlock*)this);
			}
		break;
		case DATABLOCK_TYPE_BOOL:
			{
				return (float)(bool)*((BoolBlock*)this);
			}
		break;
		case DATABLOCK_TYPE_WSTRING:
			{
				return Convert::WstringToFloat(*(wstring*)*((WstringBlock*)this));
			}
		break;
		//case DATABLOCK_TYPE_VOIDPTR:
		default:
			{
				Logger::Get()->Error(L"DataBlock: No conversion exists between float and ", Type, true);
				return 80000801.f;
			}
		break;
	}
}
DataBlock::operator float*(){
	if(Type == DATABLOCK_TYPE_ERROR){

		Logger::Get()->Error(L"DataBlock: operator float converting block of type error", Type, true);
		return NULL;
	}
	if(Type == DATABLOCK_TYPE_FLOAT){
		return (float*)*((IntBlock*)this);
	}
	switch(Type){
	//case DATABLOCK_TYPE_INT:
		case DATABLOCK_TYPE_INT:
			{
				return (float*)(int*)*((IntBlock*)this);
			}
		break;
		case DATABLOCK_TYPE_BOOL:
			{
				return (float*)(bool*)*((BoolBlock*)this);
			}
		break;
		case DATABLOCK_TYPE_WSTRING:
			{
				Logger::Get()->Error(L"Calling conversion on wstring to float: LEAKED MEMORY", 80000800, true);
				return new float(Convert::WstringToFloat(*(wstring*)*((WstringBlock*)this)));
			}
		break;
		//case DATABLOCK_TYPE_VOIDPTR:
		default:
			{
				Logger::Get()->Error(L"DataBlock: No conversion exists between float and ", Type, true);
				return NULL;
			}
		break;
	}
}

DataBlock::operator bool(){
	if(Type == DATABLOCK_TYPE_ERROR){

		Logger::Get()->Error(L"DataBlock: operator bool converting block of type error", Type, true);
		return false;
	}
	if(Type == DATABLOCK_TYPE_BOOL){
		return (bool)*((IntBlock*)this);
	}
	switch(Type){
	//case DATABLOCK_TYPE_INT:
		case DATABLOCK_TYPE_FLOAT:
			{
				return (float)*((FloatBlock*)this) != 0;
			}
		break;
		case DATABLOCK_TYPE_INT:
			{
				return (int)*((IntBlock*)this) != 0;
			}
		break;
		case DATABLOCK_TYPE_WSTRING:
			{
				return Convert::WstringFromBoolToInt(*(wstring*)*((WstringBlock*)this)) != 0;
			}
		break;
		//case DATABLOCK_TYPE_VOIDPTR:
		default:
			{
				Logger::Get()->Error(L"DataBlock: No conversion exists between bool and ", Type, true);
				return false;
			}
		break;
	}
}
DataBlock::operator bool*(){
	if(Type == DATABLOCK_TYPE_ERROR){

		Logger::Get()->Error(L"DataBlock: operator bool converting block of type error", Type, true);
		return NULL;
	}
	if(Type == DATABLOCK_TYPE_BOOL){
		return (bool*)*((IntBlock*)this);
	}
	switch(Type){
	//case DATABLOCK_TYPE_INT:
		case DATABLOCK_TYPE_FLOAT:
			{
				return (bool*)(float*)*((FloatBlock*)this);
			}
		break;
		case DATABLOCK_TYPE_INT:
			{
				return (bool*)(int*)*((IntBlock*)this);
			}
		break;
		case DATABLOCK_TYPE_WSTRING:
			{
				Logger::Get()->Error(L"Calling conversion on wstring to bool: LEAKED MEMORY", 80000800, true);
				return new bool(Convert::WstringFromBoolToInt(*(wstring*)*((WstringBlock*)this)) != 0);
			}
		break;
		//case DATABLOCK_TYPE_VOIDPTR:
		default:
			{
				Logger::Get()->Error(L"DataBlock: No conversion exists between bool and ", Type, true);
				return NULL;
			}
		break;
	}
}

DataBlock::operator wstring(){
	if(Type == DATABLOCK_TYPE_ERROR){

		Logger::Get()->Error(L"DataBlock: operator wstring converting block of type error", Type, true);
		return L"80000800";
	}
	if(Type == DATABLOCK_TYPE_WSTRING){
		return (wstring)*((WstringBlock*)this);
	}
	switch(Type){
	//case DATABLOCK_TYPE_WSTRING:
		case DATABLOCK_TYPE_FLOAT:
			{
				return Convert::ToWstring((float)*((FloatBlock*)this));
			}
		break;
		case DATABLOCK_TYPE_BOOL:
			{
				return Convert::ToWstring((float)*((BoolBlock*)this));
			}
		break;
		case DATABLOCK_TYPE_INT:
			{
				return Convert::ToWstring((float)*((IntBlock*)this));
			}
		break;
		case DATABLOCK_TYPE_VOIDPTR:
			{
				return *(wstring*)*((IntBlock*)this);
			}
		break;
		//case DATABLOCK_TYPE_VOIDPTR:
		default:
			{
				Logger::Get()->Error(L"DataBlock: No conversion exists between wstring and ", Type, true);
				return L"80000801";
			}
		break;
	}
}
DataBlock::operator wstring*(){
	if(Type == DATABLOCK_TYPE_ERROR){

		Logger::Get()->Error(L"DataBlock: operator wstring converting block of type error", Type, true);
		return NULL;
	}
	if(Type == DATABLOCK_TYPE_WSTRING){
		return (wstring*)*((WstringBlock*)this);
	}
	switch(Type){
	//case DATABLOCK_TYPE_WSTRING:
		case DATABLOCK_TYPE_FLOAT:
			{
				Logger::Get()->Error(L"Calling conversion on wstring: LEAKED MEMORY", 80000800, true);
				return new wstring(Convert::ToWstring((float)*((FloatBlock*)this)));
			}
		break;
		case DATABLOCK_TYPE_BOOL:
			{
				Logger::Get()->Error(L"Calling conversion on wstring: LEAKED MEMORY", 80000800, true);
				return new wstring(Convert::ToWstring((float)*((BoolBlock*)this)));
			}
		break;
		case DATABLOCK_TYPE_INT:
			{
				Logger::Get()->Error(L"Calling conversion on wstring: LEAKED MEMORY", 80000800, true);
				return new wstring(Convert::ToWstring((float)*((IntBlock*)this)));
			}
		break;
		case DATABLOCK_TYPE_VOIDPTR:
			{
				return (wstring*)*((IntBlock*)this);
			}
		break;
		//case DATABLOCK_TYPE_VOIDPTR:
		default:
			{
				Logger::Get()->Error(L"DataBlock: No conversion exists between wstring and ", Type, true);
				return NULL;
			}
		break;
	}
}

DataBlock::operator void*(){
	if(Type == DATABLOCK_TYPE_ERROR){

		Logger::Get()->Error(L"DataBlock: operator void* converting block of type error", Type, true);
		return NULL;
	}
	if(Type == DATABLOCK_TYPE_VOIDPTR){
		return (void*)*((VoidBlock*)this);
	}
	//switch(Type){
	////case DATABLOCK_TYPE_WSTRING:
	//	//case DATABLOCK_TYPE_FLOAT:
	//	//	{
	//	//		Logger::Get()->Error(L"Calling conversion on wstring: LEAKED MEMORY", 80000800, true);
	//	//		return new wstring(Convert::ToWstring((float)*((FloatBlock*)this)));
	//	//	}
	//	//break;
	//	//case DATABLOCK_TYPE_BOOL:
	//	//	{
	//	//		Logger::Get()->Error(L"Calling conversion on wstring: LEAKED MEMORY", 80000800, true);
	//	//		return new wstring(Convert::ToWstring((float)*((BoolBlock*)this)));
	//	//	}
	//	//break;
	//	//case DATABLOCK_TYPE_INT:
	//	//	{
	//	//		Logger::Get()->Error(L"Calling conversion on wstring: LEAKED MEMORY", 80000800, true);
	//	//		return new wstring(Convert::ToWstring((float)*((IntBlock*)this)));
	//	//	}
	//	//break;
	//	//case DATABLOCK_TYPE_VOIDPTR:
	//	//	{
	//	//		return (wstring*)*((IntBlock*)this);
	//	//	}
	//	//break;
	//	//case DATABLOCK_TYPE_VOIDPTR:
	//	default:
	//		{
	//			Logger::Get()->Error(L"DataBlock: No conversion exists between void* and ", Type, true);
	//			return NULL;
	//		}
	//	break;
	//}
	Logger::Get()->Error(L"DataBlock: No conversion exists between void* and ", Type, true);
	return NULL;
}
// --------------- IntBlock ------------------ //
IntBlock::IntBlock() : DataBlock(){
	Type = DATABLOCK_TYPE_ERROR;
	Value = NULL;
}
IntBlock::IntBlock(int value/*,int type*/) : DataBlock(DATABLOCK_TYPE_INT){
	Value = new int(value);
}
IntBlock::IntBlock(IntBlock* arg){
	Type = arg->Type;
	Value = arg->Value;
}
IntBlock::~IntBlock(){
	if(Type != DATABLOCK_TYPE_VOIDPTR)
		SAFE_DELETE(Value);
}
// ------------------------------------ //
IntBlock IntBlock::operator=(const IntBlock& arg){
	return (IntBlock(*arg.Value/*, arg.Type*/));
}
IntBlock IntBlock::operator=(int& arg){
	return (IntBlock(arg/*, DATABLOCK_TYPE_INT*/));
}

// ------------------------------------ //
IntBlock::operator int(){
	if(Value == NULL){
		Logger::Get()->Error(L"Calling conversion on non initialized DataBlock of type int", 80000800, true);
		return 80000800;
	}
	return *Value;
}
IntBlock::operator int*(){
	if(Value == NULL){
		Logger::Get()->Error(L"Calling conversion on non initialized DataBlock of type int: LEAKED MEMORY", 80000800, true);
		return new int(80000800);
	}
	return Value;
}
// ------------------------------------ //
	
// --------------- FloatBlock ------------------ //
FloatBlock::FloatBlock() : DataBlock(){
	Type = DATABLOCK_TYPE_ERROR;
	Value = NULL;
}
FloatBlock::FloatBlock(float value/*,int type*/) : DataBlock(DATABLOCK_TYPE_FLOAT){
	Value = new float(value);
}
FloatBlock::FloatBlock(FloatBlock* arg){
	Type = arg->Type;
	Value = arg->Value;
}
FloatBlock::~FloatBlock(){
	if(Type != DATABLOCK_TYPE_VOIDPTR)
		SAFE_DELETE(Value);
}
// ------------------------------------ //
FloatBlock FloatBlock::operator=(const FloatBlock& arg){
	return (FloatBlock(*arg.Value/*, arg.Type*/));
}
FloatBlock FloatBlock::operator=(float& arg){
	return (FloatBlock(arg));
}

// ------------------------------------ //
FloatBlock::operator float(){
	if(Value == NULL){
		Logger::Get()->Error(L"Calling conversion on non initialized DataBlock of type float", 80000800, true);
		return 80000800;
	}
	return *Value;
}
FloatBlock::operator float*(){
	if(Value == NULL){
		Logger::Get()->Error(L"Calling conversion on non initialized DataBlock of type float: LEAKED MEMORY", 80000800, true);
		return new float(80000800);
	}
	return Value;
}
// ------------------------------------ //
	
// --------------- BoolBlock ------------------ //
BoolBlock::BoolBlock() : DataBlock(){
	Type = DATABLOCK_TYPE_ERROR;
	Value = NULL;
}
BoolBlock::BoolBlock(bool value/*,int type*/) : DataBlock(DATABLOCK_TYPE_BOOL){
	Value = new bool(value);
}
BoolBlock::BoolBlock(BoolBlock* arg){
	Type = arg->Type;
	Value = arg->Value;
}
BoolBlock::~BoolBlock(){
	if(Type != DATABLOCK_TYPE_VOIDPTR)
		SAFE_DELETE(Value);
}
// ------------------------------------ //
BoolBlock BoolBlock::operator=(const BoolBlock& arg){
	return (BoolBlock(*arg.Value/*, arg.Type*/));
}
BoolBlock BoolBlock::operator=(bool& arg){
	return (BoolBlock(arg));
}

// ------------------------------------ //
BoolBlock::operator bool(){
	if(Value == NULL){
		Logger::Get()->Error(L"Calling conversion on non initialized DataBlock of type bool", 80000800, true);
		return false;
	}
	return *Value;
}
BoolBlock::operator bool*(){
	if(Value == NULL){
		Logger::Get()->Error(L"Calling conversion on non initialized DataBlock of type bool: LEAKED MEMORY", 80000800, true);
		return new bool(false);
	}
	return Value;
}
// ------------------------------------ //

// --------------- WstringBlock ------------------ //
WstringBlock::WstringBlock() : DataBlock(){
	Type = DATABLOCK_TYPE_ERROR;
	Value = NULL;
}
WstringBlock::WstringBlock(wstring value/*,int type*/) : DataBlock(DATABLOCK_TYPE_WSTRING){
	Value = new wstring(value);
}
WstringBlock::WstringBlock(WstringBlock* arg){
	Type = arg->Type;
	Value = arg->Value;
}
WstringBlock::~WstringBlock(){
	if(Type != DATABLOCK_TYPE_VOIDPTR)
		SAFE_DELETE(Value);
}
// ------------------------------------ //
WstringBlock WstringBlock::operator=(const WstringBlock& arg){
	return (WstringBlock(*arg.Value/*, arg.Type*/));
}
WstringBlock WstringBlock::operator=(wstring& arg){
	return (WstringBlock(arg));
}

// ------------------------------------ //
WstringBlock::operator wstring(){
	if(Value == NULL){
		Logger::Get()->Error(L"Calling conversion on non initialized DataBlock of type wstring", 80000800, true);
		return L"80000800";
	}
	return *Value;
}
WstringBlock::operator wstring*(){
	if(Value == NULL){
		Logger::Get()->Error(L"Calling conversion on non initialized DataBlock of type wstring: LEAKED MEMORY", 80000800, true);
		return new wstring(L"80000800");
	}
	return Value;
}
// ------------------------------------ //
  
// --------------- VoidBlock ------------------ //
VoidBlock::VoidBlock() : DataBlock(){
	Type = DATABLOCK_TYPE_ERROR;
	Value = NULL;
}
VoidBlock::VoidBlock(void* value/*,int type*/) : DataBlock(DATABLOCK_TYPE_VOIDPTR){
	Value = value;
}
VoidBlock::VoidBlock(VoidBlock* arg){
	Type = arg->Type;
	Value = arg->Value;
}
VoidBlock::~VoidBlock(){
	if(Type != DATABLOCK_TYPE_VOIDPTR)
		SAFE_DELETE(Value);
}
// ------------------------------------ //
VoidBlock VoidBlock::operator=(const VoidBlock& arg){
	return (VoidBlock(arg.Value/*, arg.Type*/));
}
VoidBlock VoidBlock::operator=(void* arg){
	return (VoidBlock(arg));
}

// ------------------------------------ //

VoidBlock::operator void*(){
	if(Value == NULL){
		Logger::Get()->Error(L"Calling conversion on non initialized DataBlock of type void", 80000800, true);
		return new long(80000800);
	}
	return Value;
}
// ------------------------------------ //
