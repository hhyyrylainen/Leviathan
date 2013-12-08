#ifndef LEVIATHAN_NAMEDVARS
#define LEVIATHAN_NAMEDVARS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions/ExceptionInvalidArgument.h"
#include "Common/DataStoring/DataBlock.h"
#include "../ReferenceCounted.h"

namespace Leviathan{

	class NamedVars;

	// hosts one or more VariableBlocks keeping only one name for all of them //
	class NamedVariableList{
	public:
		DLLEXPORT NamedVariableList();
		DLLEXPORT NamedVariableList(const NamedVariableList &other);
		DLLEXPORT NamedVariableList(const wstring &name, VariableBlock* value1);
		DLLEXPORT NamedVariableList(const wstring &name, const VariableBlock &val);
		// warning the vector will be wiped clean after creating new variable //
		DLLEXPORT NamedVariableList(const wstring &name, vector<VariableBlock*> values_willclear);
		DLLEXPORT NamedVariableList(wstring &line, map<wstring, shared_ptr<VariableBlock>>* predefined = NULL) throw (...);
		DLLEXPORT ~NamedVariableList();
		// ------------------------------------ //
		DLLEXPORT void SetValue(const VariableBlock &value1);
		DLLEXPORT void SetValue(VariableBlock* value1);
		DLLEXPORT void SetValue(const vector<VariableBlock*> &values);
		DLLEXPORT void SetValue(const int &nindex, const VariableBlock &valuetoset);
		DLLEXPORT void SetValue(const int &nindex, VariableBlock* valuetoset);

		DLLEXPORT VariableBlock* GetValueDirect();
		DLLEXPORT VariableBlock& GetValue() throw(...);
		DLLEXPORT VariableBlock* GetValueDirect(const int &nindex);
		DLLEXPORT VariableBlock& GetValue(const int &nindex) throw(...);
		DLLEXPORT vector<VariableBlock*>& GetValues();
		
		DLLEXPORT size_t GetVariableCount() const;

		DLLEXPORT int GetCommonType() const;
		template<class DBT>
		DLLEXPORT inline bool CanAllBeCastedToType() const{
			if(Datas.size() == 0)
				return false;

			for(size_t i = 0; i < Datas.size(); i++){
				// check this //
				if(!Datas[i]->IsConversionAllowedNonPtr<DBT>()){
					return false;
				}
			}
			// all passed, can be casted //
			return true;
		}
		template<class DBT>
		DLLEXPORT inline bool CanAllBeCastedToType(const int &startindex, const int &endindex) const{
			if(Datas.size() == 0)
				return false;
			// check would it go over //
			if((size_t)endindex >= Datas.size())
				return false;

			for(int i = startindex; i < endindex+1; i++){
				// check this //
				if(!Datas[(size_t)i]->IsConversionAllowedNonPtr<DBT>()){
					return false;
				}
			}
			// all passed, can be casted //
			return true;
		}

		DLLEXPORT int GetVariableType() const;
		DLLEXPORT int GetVariableType(const int &nindex) const;

		DLLEXPORT wstring& GetName();
		DLLEXPORT void GetName(wstring &name) const;

		DLLEXPORT void SetName(const wstring &name);
		DLLEXPORT bool CompareName(const wstring &name) const;
		// ------------------------------------ //
		DLLEXPORT wstring ToText(int WhichSeparator = 0) const;
		// process functions //
		DLLEXPORT static int ProcessDataDump(const wstring &data, vector<shared_ptr<NamedVariableList>> &vec, 
			map<wstring, shared_ptr<VariableBlock>>* predefined = NULL);
		// operators //
		DLLEXPORT NamedVariableList& operator=(const NamedVariableList &other);

		// element access operator //
		DLLEXPORT VariableBlock& operator[](const int &nindex) throw(...);

		//************************************
		// Method:    SwitchValues
		// FullName:  Leviathan::NamedVariableList::SwitchValues
		// Access:    public static 
		// Returns:   DLLEXPORT  void
		// Qualifier:
		// Parameter: NamedVariableList & receiver
		// Parameter: NamedVariableList & donator
		// Usage: Efficient way to copy new values to existing NamedVariableList, WARNING Will demolish the value that's values are copied
		//************************************
		DLLEXPORT static void SwitchValues(NamedVariableList &receiver, NamedVariableList &donator);

	private:
		// utility functions //
		inline void _DeleteAllButFirst(){

			for(size_t i = 1; i < Datas.size(); i++){

				SAFE_DELETE(Datas[i]);

			}
			if(Datas.size() > 1)
				Datas.resize(1);
		}


		vector<VariableBlock*> Datas;

		wstring Name;

		// friends //
		friend NamedVars;
	};


	// holds a vector of NamedVariableLists and provides searching functions //
	class NamedVars : public ReferenceCounted{
	public:
		DLLEXPORT NamedVars();
		DLLEXPORT NamedVars(const NamedVars &other);
		DLLEXPORT NamedVars(const wstring &datadump);
		DLLEXPORT NamedVars(const vector<shared_ptr<NamedVariableList>> &variables);
		DLLEXPORT NamedVars(shared_ptr<NamedVariableList> variable);
		DLLEXPORT ~NamedVars();
		// ------------------------------------ //
		DLLEXPORT bool SetValue(const wstring &name, const VariableBlock &value1);
		DLLEXPORT bool SetValue(const wstring &name, VariableBlock* value1);
		DLLEXPORT bool SetValue(const wstring &name, const vector<VariableBlock*> &values);

		DLLEXPORT bool SetValue(NamedVariableList &nameandvalues);

		DLLEXPORT size_t GetValueCount(const wstring &name) const;

		DLLEXPORT VariableBlock& GetValueNonConst(const wstring &name) throw(...);
		DLLEXPORT const VariableBlock* GetValue(const wstring &name) const;
		DLLEXPORT bool GetValue(const wstring &name, VariableBlock &receiver) const;
		DLLEXPORT bool GetValue(const wstring &name, const int &nindex, VariableBlock &receiver) const;
		DLLEXPORT bool GetValues(const wstring &name, vector<const VariableBlock*> &receiver) const;

		DLLEXPORT shared_ptr<NamedVariableList> GetValueDirect(const wstring &name) const;

		template<class T>
		DLLEXPORT bool GetValueAndConvertTo(const wstring &name, T &receiver) const{
			// use try block to catch all exceptions (not found and conversion fail //
			try{
				const VariableBlock* tmpblock = this->GetValue(name);
				if(tmpblock == NULL){
					return false;
				}
				if(!tmpblock->ConvertAndAssingToVariable<T>(receiver)){

					throw exception("invalid");
				}
			}
			catch(...){
				// variable not found / wrong type //
				return false;
			}
			// correct variable has been set //
			return true;
		}

		DLLEXPORT vector<VariableBlock*>* GetValues(const wstring &name) throw(...);

		// Script accessible functions //
		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(NamedVars);

		// Uses the find functions to get first value from the found value //
		// Warning: uses reference counting for return value //
		ScriptSafeVariableBlock* GetScriptCompatibleValue(string name);
		// ------------------------------------ //
		DLLEXPORT int GetVariableType(const wstring &name) const;
		DLLEXPORT int GetVariableType(unsigned int index) const;
		DLLEXPORT int GetVariableTypeOfAll(const wstring &name) const;
		DLLEXPORT int GetVariableTypeOfAll(unsigned int index) const;

		DLLEXPORT wstring& GetName(unsigned int index) throw(...);
		DLLEXPORT bool GetName(unsigned int index, wstring &name) const;

		DLLEXPORT void SetName(unsigned int index, const wstring &name);
		DLLEXPORT void SetName(const wstring &oldname, const wstring &name);

		DLLEXPORT bool CompareName(unsigned int index, const wstring &name) const;
		// ------------------------------------ //
		DLLEXPORT void AddVar(NamedVariableList* newvaluetoadd);
		DLLEXPORT void AddVar(shared_ptr<NamedVariableList> values);
		DLLEXPORT void AddVar(const wstring &name, VariableBlock* valuetosteal);
		DLLEXPORT void Remove(unsigned int index);
		DLLEXPORT void Remove(const wstring &name);
		// ------------------------------------ //
		DLLEXPORT int LoadVarsFromFile(const wstring &file);

		DLLEXPORT vector<shared_ptr<NamedVariableList>>* GetVec();
		DLLEXPORT void SetVec(vector<shared_ptr<NamedVariableList>> &vec);
		// ------------------------------------ //
		DLLEXPORT int Find(const wstring &name) const;

	private:
		vector<shared_ptr<NamedVariableList>> Variables;
	};

}

#endif