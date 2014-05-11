#ifndef LEVIATHAN_OBJECTFILEPROCESSOR
#define LEVIATHAN_OBJECTFILEPROCESSOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ObjectFile.h"

namespace Leviathan{

	//! \brief Static class for handling ObjectFiles
	class ObjectFileProcessor{
	public:
		DLLEXPORT static void Initialize();
		DLLEXPORT static void Release();

		//! \brief Reads an ObjectFile to an in-memory data structure
		DLLEXPORT static unique_ptr<ObjectFile> ProcessObjectFile(const std::wstring &file);


		//! \brief Writes an ObjectFile's data structure to a file
		//! \warning Using the process and this function will erase ALL comments, which is not optimal for files. It is recommended to only append
		//! to an existing file to keep comments intact
		//! \return True when the file has been written, false if something failed
		DLLEXPORT static bool WriteObjectFile(ObjectFile &data, const std::wstring &file);


		//! \brief Registers a new value alias for the processor
		//! \warning Calling this while files are being parsed will cause undefined behavior
		DLLEXPORT static void RegisterValue(const wstring &name, VariableBlock* valuetokeep);



		// Utility functions //


		// function to shorten value loading in many places //
		template<class T>
		DLLEXPORT static bool LoadValueFromNamedVars(NamedVars* block, const wstring &varname, T &receiver, const T &defaultvalue, bool ReportError
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

				return false;
			}
			return true;
		}
		// function to call the one before //
		template<class T>
		static FORCE_INLINE bool LoadValueFromNamedVars(NamedVars &block, const wstring &varname, T &receiver, const T &defaultvalue, bool ReportError
			= false, const wstring &errorprefix = L"")
		{
				return LoadValueFromNamedVars<T>(&block, varname, receiver, defaultvalue, ReportError, errorprefix);
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
		static FORCE_INLINE void LoadMultiPartValueFromNamedVars(NamedVars &block, const wstring &varname, RType &receiver, const RType &defaultvalue, bool ReportError
			= false, const wstring &errorprefix = L"")
		{
			return LoadMultiPartValueFromNamedVars<RType, SingleType, VarCount>(&block, varname, receiver, defaultvalue, ReportError, errorprefix);
		}

	private:

		//! \brief Handling function for NamedVariables
		static shared_ptr<NamedVariableList> TryToLoadNamedVariables(const wstring &file, StringIterator &itr, const string &preceeding);

		//! \brief Handling function for template definitions and instantiations
		static bool TryToHandleTemplate(const wstring &file, StringIterator &itr, ObjectFile &obj, const string &preceeding);

		//! \brief Handling function for loading whole objects
		static shared_ptr<ObjectFileObject> TryToLoadObject(const wstring &file, StringIterator &itr, ObjectFile &obj, const string &preceeding);


		// Object loading split into parts //
		//! \brief Loading variable lists
		static bool TryToLoadVariableList(const wstring &file, StringIterator &itr, ObjectFileObject &obj, size_t startline);


		//! \brief Loading text blocks
		static bool TryToLoadTextBlock(const wstring &file, StringIterator &itr, ObjectFileObject &obj, size_t startline);


		//! \brief Loading script blocks
		static bool TryToLoadScriptBlock(const wstring &file, StringIterator &itr, ObjectFileObject &obj, size_t startline);



		// private constructor to prevent instantiating //
		ObjectFileProcessor();
		~ObjectFileProcessor();

		static map<wstring, shared_ptr<VariableBlock>> RegisteredValues;
	};

}
#endif
