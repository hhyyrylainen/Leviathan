#ifndef LEVIATHAN_OBJECTFILEOBJECT
#define LEVIATHAN_OBJECTFILEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ObjectFiles/ObjectFileList.h"
#include "ObjectFiles/ObjectFileTextBlock.h"
#include "Script/ScriptScript.h"

namespace Leviathan{

	class ObjectFileObject : public Object{
	public:
		DLLEXPORT ObjectFileObject::ObjectFileObject(const wstring &name, const wstring &typesname, vector<shared_ptr<wstring>> prefix = 
			vector<shared_ptr<wstring>>());
		DLLEXPORT ObjectFileObject::~ObjectFileObject();

		wstring Name;
		wstring TName;

		std::vector<shared_ptr<wstring>> Prefixes;
		std::vector<ObjectFileList*> Contents;
		std::vector<ObjectFileTextBlock*> TextBlocks;

		shared_ptr<ScriptScript> Script;
	};

}
#endif