#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILE_OBJECT
#include "ObjectFileObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ObjectFileObject::ObjectFileObject(){
	Script = NULL;
}
ObjectFileObject::ObjectFileObject(wstring name, int type, wstring typenam){
	//BaseType = basetype;
	Name = name;
	TName = typenam;
	Type = type;
	Script = NULL;
}

ObjectFileObject::~ObjectFileObject(){
	// release some stuff //
	while(Prefixes.size() != 0){
		SAFE_DELETE(Prefixes[0]);
		Prefixes.erase(Prefixes.begin());
	}
	while(Contents.size() != 0){
		SAFE_DELETE(Contents[0]);
		Contents.erase(Contents.begin());
	}

	SAFE_DELETE_VECTOR(TextBlocks);
}
// ------------------------------------ //
//bool ObjectFileObject::ContainsScript(wstring name){
//	for(int i = 0; i < Scripts.size(); i++){
//		if(Scripts[i]->Name == name)
//			return true;
//	}
//	return false;
//}
// ------------------------------------ //
ScriptObject* ObjectFileObject::CreateScriptObjectFromThis(int BaseType, int Overridetype){
	ScriptObject* obj;
	if(Overridetype != -1){
		obj = new ScriptObject(Name, BaseType, Overridetype, TName);

	} else {
		obj = new ScriptObject(Name, BaseType, Type, TName);
	}
	// re allocate all the data //

	// script object can just be copied, because it uses smart pointers //
	obj->Script = this->Script;



	vector<wstring*> prfx;
	vector<ScriptList*> cnts;

	for(unsigned int i = 0; i < Prefixes.size(); i++){
		prfx.push_back(new wstring(*Prefixes[i]));
	}
	for(unsigned int i = 0; i < Contents.size(); i++){
		cnts.push_back(Contents[i]->AllocateNewListFromData());
	}

	obj->Prefixes = prfx;
	obj->Contents = cnts;

	return obj;
}
ScriptObject* ObjectFileObject::CreateScriptObjectAndDeleteThis(int BaseType, int Overridetype){
	ScriptObject* obj;
	if(Overridetype != -1){
		obj = new ScriptObject(Name, BaseType, Overridetype, TName);

	} else {
		obj = new ScriptObject(Name, BaseType, Type, TName);
	}
	// just move pointers and then release this object //

	// script object can just be copied, because it uses smart pointers //
	obj->Script = (this->Script);
	// reset script smart pointer so that it doesn't accidentally get deleted //
	Script.reset();

	vector<ScriptList*> cnts;

	for(unsigned int i = 0; i < Contents.size(); i++){
		cnts.push_back(Contents[i]->AllocateNewListFromData());
	}

	obj->Prefixes = Prefixes;
	Prefixes.clear();
	obj->Contents = cnts;

	// self releasing //
	this->~ObjectFileObject();
	
	delete this;
	//this = NULL;

	return obj;
}
// ------------------------------------ //

// ------------------------------------ //