#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_OBJECT
#include "ScriptObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ScriptObject::ScriptObject(){
	Script = NULL;
}
ScriptObject::ScriptObject(wstring name, int basetype, int type, wstring typenam) : Contents(), Prefixes(){
	BaseType = basetype;
	Name =  wstring(name);
	TName =  wstring(typenam);
	Type = type;
	Script = NULL;
}

ScriptObject::~ScriptObject(){
	// release some stuff //
	Prefixes.clear();
	Contents.clear();
}
// ------------------------------------ //
//bool ScriptObject::ContainsScript(wstring name){
//	for(int i = 0; i < Scripts.size(); i++){
//		if(Scripts[i]->Name == name)
//			return true;
//	}
//	return false;
//}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //