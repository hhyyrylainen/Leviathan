#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_CALLER
#include "ScriptCaller.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
//#include "GlobalCallable.h"
#include "ScriptException.h"

ScriptCaller::ScriptCaller(){

}
ScriptCaller::ScriptCaller(bool justrueforglobalcall){
	if(!justrueforglobalcall)
		// this shouldn't be hit ever //
		DEBUG_BREAK;
	Global = this;

	// define functions that are exposed to scripts //
	//this->RegisterFunction(L"Data::GetTickCount", GetTickCount);
	//this->RegisterFunction(L"Logger::Info", LogPrintInfo);
	//this->RegisterFunction(L"Logger::Error", LogPrintError);
}
ScriptCaller::~ScriptCaller(){
	while(FunctionNames.size() != 0){
		delete FunctionNames[0];
		FunctionNames.erase(FunctionNames.begin());
	}
}

ScriptCaller* ScriptCaller::Global = NULL;
// ------------------------------------ //
ScriptCaller* ScriptCaller::GetGlobal(){
	return Global;
}
// ------------------------------------ //
int ScriptCaller::SearchFunctions(wstring &name){
	for(unsigned int i = 0; i < FunctionNames.size(); i++){
		if((*FunctionNames[i]) == name){
			return i;
		}
	}
	return -1;
}
void ScriptCaller::RemoveFunction(unsigned int index){
	ARR_INDEX_CHECK(index, FunctionNames.size()){

		Logger::Get()->Error(L"ScriptCaller: trying to remove object with invalid index", index);
		return;
	}
	delete FunctionNames[index];
	FunctionNames.erase(FunctionNames.begin()+index);
	Funcptrs.erase(Funcptrs.begin()+index);
}
void ScriptCaller::RegisterFunction(wstring name, CallerFunctionPointerType func){
	FunctionNames.push_back(new wstring(name));
	Funcptrs.push_back(func);
}
// ------------------------------------ //
int ScriptCaller::CallFunction(wstring name, vector<ScriptArguement*>* args, int index){
	if(index == -1){
		// search
		index = SearchFunctions(name);
	}
	ARR_INDEX_CHECKINV((unsigned int)index, FunctionNames.size()){

		Logger::Get()->Error(L"ScriptCaller: trying to call object with invalid index", index);
		return 404;
	}
	try {
		return Funcptrs[index](args);
	}
	catch(ScriptException e){
		Logger::Get()->Error(L"ScriptCaller: calling function: "+name+L" exception: "+Convert::IntToWstring(e.ErrorCode)+L" "+e.Message+L" action "+Convert::IntToWstring(e.ActionValue)+L" from "+e.Source, true);
	}
	return 5;
}
// ------------------------------------ //