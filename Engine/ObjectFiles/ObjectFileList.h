#ifndef LEVIATHAN_OBJECTFILE_LIST
#define LEVIATHAN_OBJECTFILE_LIST
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/DataStoring/NamedVars.h"

namespace Leviathan{

	class ObjectFileList : public Object{
	public:
		DLLEXPORT ObjectFileList::ObjectFileList();
		DLLEXPORT ObjectFileList::ObjectFileList(const wstring &name);
		DLLEXPORT ObjectFileList::~ObjectFileList();


		wstring Name;
		NamedVars/***/ Variables;
		vector<wstring*> Lines; // for storing plain text //
	};

}
#endif