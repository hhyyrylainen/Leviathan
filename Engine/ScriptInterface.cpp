#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_INTERFACE
#include "ScriptInterface.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "FileSystem.h"

ScriptInterface::ScriptInterface(){
	GlobalCaller = NULL;
	ScriptRunner = NULL;
}
ScriptInterface::~ScriptInterface(){

}

ScriptInterface* ScriptInterface::staticaccess = NULL;
ScriptInterface* ScriptInterface::Get(){ return staticaccess; };
// ------------------------------------ //
bool ScriptInterface::Init(){
	GlobalCaller = new ScriptCaller(true);
	ScriptRunner = new ScriptExecutor();
	if(!ScriptRunner->Init()){

		Logger::Get()->Error(L"ScriptInterface: failed to init script runner",true);
		return false;
	}

	staticaccess = this;

	return true;
}
void ScriptInterface::Release(){
	SAFE_DELETE(GlobalCaller);
	SAFE_RELEASEDEL(ScriptRunner);

	while(Managed.size() != 0){
		// TODO: use script executor to see if on delete exists //
		SAFE_DELETE(Managed[0]);

	}
}
// ------------------------------------ //




// ------------------------------------ //
void ScriptInterface::TakeOwnerShip(ScriptObject* obj){
	Managed.push_back(obj);
} // takes the object as manageable //
void ScriptInterface::RemoveOwnerShip(ScriptObject* obj, int index){
	if(index != -1){
		// TODO: use script executor to see if on delete exists //
		Managed.erase(Managed.begin()+index);
		return;
	}
	index = SearchScript(L"", obj);
	ARR_INDEX_CHECK((unsigned int)index, Managed.size()){

		Logger::Get()->Error(L"ScriptInterface trying to unmanage script with invalid index", index, true);
		return;
	}
	Managed.erase(Managed.begin()+index);
}
void ScriptInterface::ReleaseScript(unsigned int index){
	ARR_INDEX_CHECK(index, Managed.size()){

		Logger::Get()->Error(L"ScriptInterface trying to release script with invalid index", index, true);
		return;
	}
	// TODO: use script executor to see if on delete exists //
	SAFE_DELETE(Managed[index]);
}
int ScriptInterface::SearchScript(wstring name, ScriptObject* obj){
	if(obj != NULL){
		// search by pointer 
		for(unsigned int i = 0; i < Managed.size(); i++){
			if(Managed[i] == obj)
				return i;
		}
		return -1;
	}
	for(unsigned int i = 0; i < Managed.size(); i++){
		// TODO: implement search
	}
	return -1;
}
int ScriptInterface::RunManagedScript(int index){
	return 0;
}
// ------------------------------------ //
shared_ptr<ScriptArguement> ScriptInterface::ExecuteScript(ScriptObject* obj, wstring entrypoint, vector<ScriptNamedArguement*> Parameters, ScriptCaller* callconv, bool FullDecl){

	vector<ScriptCaller*> callobjs;
	if(callconv == NULL){
		callconv = this->GlobalCaller;
		callobjs.push_back(callconv);
	} else {

		callobjs.push_back(callconv);
		callobjs.push_back(GlobalCaller);
	}


	return ScriptRunner->RunScript(obj->Script.get(), callobjs, obj->Varss, Parameters, true, entrypoint, true, FullDecl);
}
shared_ptr<ScriptArguement> ScriptInterface::ExecuteScript(ScriptScript* script, vector<shared_ptr<ScriptVariableHolder>> vars, wstring entrypoint, vector<ScriptNamedArguement*> Parameters, ScriptCaller* callconv, bool FullDecl){

	vector<ScriptCaller*> callobjs;
	if(callconv == NULL){
		callconv = this->GlobalCaller;
		callobjs.push_back(callconv);
	} else {

		callobjs.push_back(callconv);
		callobjs.push_back(GlobalCaller);
	}
	

	return ScriptRunner->RunScript(script, callobjs, vars, Parameters, true, entrypoint, true, FullDecl);
}

shared_ptr<ScriptArguement> ScriptInterface::ExecuteIfExistsScript(ScriptObject* obj, wstring entrypoint, vector<ScriptNamedArguement*> Parameters, ScriptCaller* callconv, bool FullDecl){
	vector<ScriptCaller*> callobjs;
	if(callconv == NULL){
		callconv = this->GlobalCaller;
		callobjs.push_back(callconv);
	} else {

		callobjs.push_back(callconv);
		callobjs.push_back(GlobalCaller);
	}


	return ScriptRunner->RunScript(obj->Script.get(), callobjs, obj->Varss, Parameters, true, entrypoint, false, FullDecl);
}