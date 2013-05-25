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

	Prefixes.clear();

	SAFE_DELETE_VECTOR(Contents);
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
	unique_ptr<ScriptObject> obj;
	if(Overridetype != -1){
		obj = unique_ptr<ScriptObject>(new ScriptObject(Name, BaseType, Overridetype, TName));

	} else {
		obj = unique_ptr<ScriptObject>(new ScriptObject(Name, BaseType, Type, TName));
	}
	// re allocate all the data //

	// script object can just be copied, because it uses smart pointers //
	obj->Script = this->Script;



	vector<shared_ptr<wstring>> prfx;
	vector<shared_ptr<ScriptList>> cnts;

	for(unsigned int i = 0; i < Prefixes.size(); i++){
		prfx.push_back(shared_ptr<wstring>(new wstring(*Prefixes[i])));
	}
	for(unsigned int i = 0; i < Contents.size(); i++){
		cnts.push_back(shared_ptr<ScriptList>(Contents[i]->AllocateNewListFromData()));
	}

	obj->Prefixes = prfx;
	obj->Contents = cnts;
	// reset smart pointer //
	ScriptObject* tempptr = obj.get();
	obj.release();
	return tempptr;
}
ScriptObject* ObjectFileObject::CreateScriptObjectAndReleaseThis(int BaseType, int Overridetype){
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

	vector<shared_ptr<ScriptList>> cnts;

	for(unsigned int i = 0; i < Contents.size(); i++){
		cnts.push_back(shared_ptr<ScriptList>(Contents[i]->AllocateNewListFromData()));
		SAFE_DELETE(Contents[i]);
	}
	obj->Prefixes = Prefixes;
	obj->Contents = cnts;
	// don't forget to let go of memory //
	Prefixes.clear();

	return obj;
}
// ------------------------------------ //

// ------------------------------------ //