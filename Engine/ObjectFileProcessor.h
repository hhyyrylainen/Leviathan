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


		DLLEXPORT static void RegisterValue(const wstring &name, VariableBlock* valuetokeep);

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
		// function to call the one before //
		template<class T>
		static __forceinline void LoadValueFromNamedVars(NamedVars &block, const wstring &varname, T &receiver, const T &defaultvalue, bool ReportError 
			= false, const wstring &errorprefix = L""){
				LoadValueFromNamedVars<T>(&block, varname, receiver, defaultvalue, ReportError, errorprefix);
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
			// call assigning function //
			int varindex = 0;
			LoadMultiPartValueFromNamedVariableList<RType, SingleType, VarCount>(curvalues.get(), varindex, receiver, defaultvalue, ReportError, 
				errorprefix);

		}

		template<class RType, class SingleType, int VarCount>
		DLLEXPORT static bool LoadMultiPartValueFromNamedVariableList(NamedVariableList* block, int &valuestartindex, RType &receiver, 
			const RType &defaultvalue, bool ReportError = false, const wstring &errorprefix = L"")
		{
			// make sure that size is right and types are correct //
			if(block->GetVariableCount()-valuestartindex < VarCount || !block->CanAllBeCastedToType<SingleType>(valuestartindex, valuestartindex+VarCount-1)){
				// not enough values / wrong types //
				if(ReportError){
					Logger::Get()->Error(errorprefix+L" invalid variable "+block->GetName()+L", not enough values ("+Convert::ToWstring<int>(VarCount)
						+L" needed) or wrong types");
				}
				// set as default //
				receiver = defaultvalue;
				return false;
			}

			// iterate over how many are wanted and assign //
			for(int i = 0; i < VarCount; i++){

				// convert and set //
				receiver[i] = (SingleType)block->GetValue(valuestartindex+i);

			}
			// values copied //
			// increment the index before returning //
			valuestartindex += VarCount;

			return true;
		}

		// function to call the one before //
		template<class RType, class SingleType, int VarCount>
		static __forceinline void LoadMultiPartValueFromNamedVars(NamedVars &block, const wstring &varname, RType &receiver, const RType &defaultvalue, bool ReportError 
			= false, const wstring &errorprefix = L"")
		{
			return LoadMultiPartValueFromNamedVars<RType, SingleType, VarCount>(&block, varname, receiver, defaultvalue, ReportError, errorprefix);
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

		// private constructor to prevent instantiating //
		ObjectFileProcessor::ObjectFileProcessor();
		ObjectFileProcessor::~ObjectFileProcessor();

		static map<wstring, shared_ptr<VariableBlock>> RegisteredValues;
	};

}
#endif