#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_ARGUEMENT
#include "ScriptArguement.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
// -----------------------------   ScriptArguement   ----------------------------- //

ScriptArguement::ScriptArguement(DataBlock *data, int type, bool delonrelease){
	Type = type;

	Val = data;
	Delete = delonrelease;
}

ScriptArguement::~ScriptArguement(){
	if((Delete)){
		SAFE_DELETE(Val);
	}
}

ScriptArguement::ScriptArguement(const ScriptArguement& arg){
	Type = arg.Type;

	Val = arg.Val;
	Delete = arg.Delete;
}

// ------------------------------------ //

ScriptArguement ScriptArguement::operator=(const ScriptArguement& arg){
	return (ScriptArguement(arg.Val, arg.Type, arg.Delete));
}
ScriptArguement::operator int(){
	if((Type == DATABLOCK_TYPE_FLOAT) | (Type == DATABLOCK_TYPE_BOOL) | (Type == DATABLOCK_TYPE_INT)){
		// cast datablock //
		return (int)*Val;
	}
	Logger::Get()->Error(L"ScriptArguement typecast error casting type "+Convert::IntToWstring(Type)+L" to int", (int)Val, true);
	return 80000801;
}
ScriptArguement::operator int*(){
	if((Type == DATABLOCK_TYPE_FLOAT) | (Type == DATABLOCK_TYPE_BOOL) | (Type == DATABLOCK_TYPE_INT)){
		// cast datablock //
		return (int*)*Val;
	}
	Logger::Get()->Error(L"ScriptArguement typecast error casting type "+Convert::IntToWstring(Type)+L" to int", (int)Val, true);
	return NULL;
}

ScriptArguement::operator float(){
	if((Type == DATABLOCK_TYPE_FLOAT) | (Type == DATABLOCK_TYPE_BOOL) | (Type == DATABLOCK_TYPE_INT)){
		// cast datablock //
		return (float)*Val;
	}
	Logger::Get()->Error(L"ScriptArguement typecast error casting type "+Convert::IntToWstring(Type)+L" to float", (int)Val, true);
	return 80000801.f;
}
ScriptArguement::operator float*(){
	if((Type == DATABLOCK_TYPE_FLOAT) | (Type == DATABLOCK_TYPE_BOOL) | (Type == DATABLOCK_TYPE_INT)){
		// cast datablock //
		return (float*)*Val;
	}
	Logger::Get()->Error(L"ScriptArguement typecast error casting type "+Convert::IntToWstring(Type)+L" to float", (int)Val, true);
	return NULL;
}

ScriptArguement::operator bool(){
	if((Type == DATABLOCK_TYPE_FLOAT) | (Type == DATABLOCK_TYPE_BOOL) | (Type == DATABLOCK_TYPE_INT)){
		// cast datablock //
		return (bool)*Val;
	}
	Logger::Get()->Error(L"ScriptArguement typecast error casting type "+Convert::IntToWstring(Type)+L" to bool", (int)Val, true);
	return false;
}
ScriptArguement::operator bool*(){
	if((Type == DATABLOCK_TYPE_FLOAT) | (Type == DATABLOCK_TYPE_BOOL) | (Type == DATABLOCK_TYPE_INT)){
		// cast datablock //
		return (bool*)*Val;
	}
	Logger::Get()->Error(L"ScriptArguement typecast error casting type "+Convert::IntToWstring(Type)+L" to bool", (int)Val, true);
	return NULL;
}

ScriptArguement::operator wstring*(){
	if(Type == DATABLOCK_TYPE_WSTRING){
		// cast datablock //
		return (wstring*)*Val;
	}
	Logger::Get()->Error(L"ScriptArguement typecast error casting type "+Convert::IntToWstring(Type)+L" to wstring*", (int)Val, true);
	return new wstring(L"");
}

ScriptArguement::operator wstring(){
	if(Type == DATABLOCK_TYPE_WSTRING){
		// cast datablock //
		return (wstring)*Val;
	}
	Logger::Get()->Error(L"ScriptArguement typecast error casting type "+Convert::IntToWstring(Type)+L" to wstring", (int)Val, true);
	return wstring(L"");
}

ScriptArguement::operator void*(){
	if(Type == DATABLOCK_TYPE_VOIDPTR){
		// cast datablock //
		return (void*)*Val;
	}
	Logger::Get()->Error(L"ScriptArguement typecast error casting type "+Convert::IntToWstring(Type)+L" to void*", (int)Val, true);
	return NULL;
}

// ------------------------------------ //

void ScriptArguement::SetValue(bool deleteold, DataBlock *data, int type, bool delonrelease){
	if(deleteold & delonrelease){
		SAFE_DELETE(Val);
	}
#ifdef _DEBUG
	else {

		Logger::Get()->Info(L"Possible memory leak: ScriptArguement::SetValue old not set for deletion");
	}
#endif
	Type = type;

	Val = data;
	Delete = delonrelease;
}


// ----------------------------- ScriptNamedArguement ----------------------------- //

ScriptNamedArguement::ScriptNamedArguement(const wstring &name, DataBlock* data, int type, bool modifiable, bool delonrelease) : ScriptArguement(data, type, delonrelease){
	Name = name;
	Modifiable = modifiable;
}


ScriptNamedArguement::~ScriptNamedArguement(){

}


ScriptNamedArguement::ScriptNamedArguement(const ScriptNamedArguement& arg) : ScriptArguement(DataBlock::CopyConstructor(arg.Val), arg.Type, arg.Delete){

}
// ------------------------------------ //

ScriptNamedArguement ScriptNamedArguement::operator=(const ScriptNamedArguement& arg){
	return ScriptNamedArguement(arg.Name, DataBlock::CopyConstructor(arg.Val), arg.Type, true, arg.Delete);
}


ScriptNamedArguement ScriptNamedArguement::operator=(const ScriptArguement& arg){
	return ScriptNamedArguement(this->Name, DataBlock::CopyConstructor(arg.Val), arg.Type, true, arg.Delete);
}
// ------------------------------------ //

void ScriptNamedArguement::SetValue(bool deleteold, const wstring &name, DataBlock* data, int type, bool modifiable, bool delonrelease){
	if(deleteold & delonrelease){
		SAFE_DELETE(Val);
	}
#ifdef _DEBUG
	else {

		Logger::Get()->Info(L"Possible memory leak: ScriptArguement::SetValue old not set for deletion");
	}
#endif
	Type = type;

	Val = data;
	Delete = delonrelease;

	Name = wstring(name);
	Modifiable = modifiable;
}
// ------------------------------------ //