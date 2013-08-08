#ifndef LEVIATHAN_OBJECTFILEOBJECT
#define LEVIATHAN_OBJECTFILEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ObjectFileList.h"
#include "ObjectFileTextBlock.h"
#include "ScriptScript.h"

namespace Leviathan{

	class ObjectFileObject : public Object{
	public:
		DLLEXPORT ObjectFileObject::ObjectFileObject(const wstring &name, const wstring &typesname);
		DLLEXPORT ObjectFileObject::~ObjectFileObject();

		wstring Name;
		wstring TName;

		vector<wstring*> Prefixes;
		vector<ObjectFileList*> Contents;
		vector<ObjectFileTextBlock*> TextBlocks;

		shared_ptr<ScriptScript> Script;
	};

}
#endif