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
		static shared_ptr<ObjectFileObject> ReadObjectBlock(wifstream &reader, wstring firstline, int &Line, const wstring& sourcefile);

		// ------------------------- //
		static vector<IntWstring*> ObjectTypes;

		// private constructor to prevent instantiating //
		ObjectFileProcessor::ObjectFileProcessor();
		ObjectFileProcessor::~ObjectFileProcessor();

		static vector<IntWstring*> RegisteredValues;
	};

}
#endif