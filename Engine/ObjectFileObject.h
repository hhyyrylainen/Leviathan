#ifndef LEVIATHAN_OBJECTFILE_OBJECT
#define LEVIATHAN_OBJECTFILE_OBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ScriptObject.h"
#include "ObjectFileList.h"
#include "ObjectFileTextBlock.h"
#include "ScriptScript.h"

namespace Leviathan{

	class ObjectFileObject : public Object{
	public:
		DLLEXPORT ObjectFileObject::ObjectFileObject();
		DLLEXPORT ObjectFileObject::ObjectFileObject(wstring name, int type, wstring typenam);
		DLLEXPORT ObjectFileObject::~ObjectFileObject();

		DLLEXPORT ScriptObject* CreateScriptObjectFromThis(int BaseType, int Overridetype = -1);
		DLLEXPORT ScriptObject* CreateScriptObjectAndReleaseThis(int BaseType, int Overridetype = -1);

		//DLLEXPORT bool ContainsScript(wstring name); doesn't work anymore

		//int BaseType;
		wstring Name;
		wstring TName;
		int Type;

		vector<shared_ptr<wstring>> Prefixes;
		vector<ObjectFileList*> Contents;
		vector<ObjectFileTextBlock*> TextBlocks;


		//vector<ScriptScript*> Scripts;
		shared_ptr<ScriptScript> Script;



	};

}
#endif