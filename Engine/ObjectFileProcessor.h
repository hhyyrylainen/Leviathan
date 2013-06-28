#ifndef LEVIATHAN_OBJECTFILEPROCESSOR
#define LEVIATHAN_OBJECTFILEPROCESSOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ScriptInterface.h"
#include "ObjectFileObject.h"
#include "TimingMonitor.h"
#include "WstringIterator.h"
#include "LineTokenizer.h"
#include "DataBlock.h"

namespace Leviathan{

	class ObjectFileProcessor/* : public Object*/{
	public:
		DLLEXPORT static void Initialize();
		DLLEXPORT static void Release();
		DLLEXPORT static vector<shared_ptr<ObjectFileObject>> ProcessObjectFile(const wstring &file, vector<shared_ptr<NamedVar>> &HeaderVars);

		DLLEXPORT static void RegisterObjectType(wstring name, int value);
		DLLEXPORT static int GetObjectTypeID(wstring &name);

		DLLEXPORT static void RegisterValue(const wstring &signature, int value);

		DLLEXPORT static int WriteObjectFile(vector<shared_ptr<ObjectFileObject>> &objects, const wstring &file, vector<shared_ptr<NamedVar>> &headervars,bool UseBinary = false);

	private:
		static shared_ptr<ObjectFileObject> ReadObjectBlock(UINT &Line, vector<wstring> &Lines, const wstring& sourcefile);
		// handling object blocks //
		static bool ProcessObjectFileBlockListBlock(UINT &Line, vector<wstring> &Lines, const wstring& sourcefile, int &Level, 
			shared_ptr<ObjectFileObject> obj, int &Handleindex, WstringIterator &itr);
		static bool ProcessObjectFileBlockScriptBlock(UINT &Line, vector<wstring> &Lines, const wstring& sourcefile, int &Level, 
			shared_ptr<ObjectFileObject> obj, int &Handleindex, WstringIterator &itr);
		static bool ProcessObjectFileBlockTextBlock(UINT &Line, vector<wstring> &Lines, const wstring& sourcefile, int &Level, 
			shared_ptr<ObjectFileObject> obj, int &Handleindex, WstringIterator &itr);


		// ------------------------- //
		static vector<IntWstring*> ObjectTypes;

		// private constructor to prevent instantiating //
		ObjectFileProcessor::ObjectFileProcessor();
		ObjectFileProcessor::~ObjectFileProcessor();

		static vector<IntWstring*> RegisteredValues;
	};

}
#endif