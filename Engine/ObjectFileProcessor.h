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
		DLLEXPORT static vector<shared_ptr<ObjectFileObject>> ProcessObjectFile(const wstring &file, vector<shared_ptr<NamedVariableList>> &HeaderVars);

		DLLEXPORT static void RegisterObjectType(wstring name, int value);
		DLLEXPORT static int GetObjectTypeID(wstring &name);

		DLLEXPORT static void RegisterValue(NamedVariableBlock* valuetokeep);

		DLLEXPORT static int WriteObjectFile(vector<shared_ptr<ObjectFileObject>> &objects, const wstring &file, vector<shared_ptr<NamedVariableList>> &headervars,bool UseBinary = false);

		// static loading utility //

		// function to shorten value loading in many places //
		template<class T>
		DLLEXPORT static void LoadValueFromNamedVars(NamedVars* block, const wstring &varname, T &receiver, const T &defaultvalue, bool ReportError 
			= false, const wstring &errorprefix = L"")
		{
			// try to get value and convert to receiver //
			if(!block->GetValueAndConvertTo<T>(varname, receiver)){
				// variable not found / wrong type //
				// report error if wanted //
				if(ReportError)
					Logger::Get()->Error(errorprefix+L" invalid variable "+varname+L", not found/wrong type");
				// set value to provided default //
				receiver = defaultvalue;
			}
		}

		template<class RType, class SingleType, int VarCount>
		DLLEXPORT static void LoadMultiPartValueFromNamedVars(NamedVars* block, const wstring &varname, RType &receiver, const RType &defaultvalue, bool ReportError 
			= false, const wstring &errorprefix = L"")
		{
			// get pointer to value list //
			shared_ptr<NamedVariableList> curvalues = block->GetValueDirect(varname);

			if(curvalues.get() == NULL){
				// not found //
				if(ReportError)
					Logger::Get()->Error(errorprefix+L" invalid variable "+varname+L", not found");
				// set as default //
				receiver = defaultvalue;
				return;
			}

			// make sure that size is right and types are correct //
			if(curvalues->GetVariableCount() < VarCount || !curvalues->CanAllBeCastedToType<SingleType>()){
				// not enough values / wrong types //
				if(ReportError){
					Logger::Get()->Error(errorprefix+L" invalid variable "+varname+L", not enough values ("+Convert::ToWstring<int>(VarCount)
						+L" needed) or wrong types");
				}
				// set as default //
				receiver = defaultvalue;
				return;
			}

			// iterate over how many are wanted and assign //
			for(int i = 0; i < VarCount; i++){

				// convert and set //
				receiver[i] = (SingleType)curvalues->GetValue(i);

			}
			// values copied //
		}

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

		static vector<const NamedVariableBlock*> RegisteredValues;
	};

}
#endif