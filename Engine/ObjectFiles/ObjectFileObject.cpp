#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILEOBJECT
#include "ObjectFileObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ObjectFileObject::ObjectFileObject(const wstring &name, const wstring &typesname, vector<shared_ptr<wstring>> 
	prefix /*= vector<shared_ptr<wstring>>()*/) : Name(name), TName(typesname), Script(NULL), Prefixes(prefix)
{

}

ObjectFileObject::~ObjectFileObject(){
	// release held memory //
	Prefixes.clear();
	SAFE_DELETE_VECTOR(Contents);
	SAFE_DELETE_VECTOR(TextBlocks);
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //