#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILEOBJECT
#include "ObjectFileObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ObjectFileObject::ObjectFileObject(const wstring &name, const wstring &typesname) : Name(name), TName(typesname), Script(NULL){

}

ObjectFileObject::~ObjectFileObject(){
	// release held memory //
	SAFE_DELETE_VECTOR(Prefixes);
	SAFE_DELETE_VECTOR(Contents);
	SAFE_DELETE_VECTOR(TextBlocks);
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //