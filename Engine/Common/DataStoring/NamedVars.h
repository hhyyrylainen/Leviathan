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
#include "SFML/Network/Packet.hpp"
#include "Common/ThreadSafe.h"

namespace Leviathan{

	//! \brief hosts one or more VariableBlocks keeping only one name for all of them
	class NamedVariableList{
		friend NamedVars;
	public:
		DLLEXPORT NamedVariableList();
		DLLEXPORT NamedVariableList(const NamedVariableList &other);
		DLLEXPORT NamedVariableList(const wstring &name, VariableBlock* value1);
		DLLEXPORT NamedVariableList(const wstring &name, const VariableBlock &val);

		//! \brief For receiving NamedVariableLists through the network
		DLLEXPORT NamedVariableList(sf::Packet &packet);

		//! \warning the vector will be wiped clean after creating new variable
		DLLEXPORT NamedVariableList(const wstring &name, vector<VariableBlock*> values_willclear);
		DLLEXPORT NamedVariableList(wstring &line, map<wstring, shared_ptr<VariableBlock>>* predefined = NULL) THROWS;
		DLLEXPORT NamedVariableList(const wstring &name, const wstring &valuestr, map<wstring,
            shared_ptr<VariableBlock>>* predefined = NULL) THROWS;

		//! \brief Helper function for constructing values
		DLLEXPORT void ConstructValuesForObject(const wstring &variablestr, map<wstring,
            shared_ptr<VariableBlock>>* predefined) THROWS;

		DLLEXPORT ~NamedVariableList();
		// ------------------------------------ //
		DLLEXPORT void SetValue(const VariableBlock &value1);
		DLLEXPORT void SetValue(VariableBlock* value1);
		DLLEXPORT void SetValue(const vector<VariableBlock*> &values);
		DLLEXPORT void SetValue(const int &nindex, const VariableBlock &valuetoset);
		DLLEXPORT void SetValue(const int &nindex, VariableBlock* valuetoset);

		DLLEXPORT VariableBlock* GetValueDirect();
		DLLEXPORT VariableBlock& GetValue() THROWS;
		DLLEXPORT VariableBlock* GetValueDirect(const int &nindex);
		DLLEXPORT VariableBlock& GetValue(const int &nindex) THROWS;
		DLLEXPORT vector<VariableBlock*>& GetValues();

		DLLEXPORT size_t GetVariableCount() const;

		//! \brief For passing NamedVariableLists to other instances through the network
		DLLEXPORT void AddToPacket(sf::Packet &packet) const;


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

		//! Compare values extensively
		//!
		//! If this returns true then the values are the same and assignment would have no visible effect.
		DLLEXPORT bool operator==(const NamedVariableList &other) const;


		// element access operator //
		DLLEXPORT VariableBlock& operator[](const int &nindex) THROWS;

		//! \brief Switches values to a new instance
		DLLEXPORT static void SwitchValues(NamedVariableList &receiver, NamedVariableList &donator);

	private:

		//! Data
		vector<VariableBlock*> Datas;

		//! Name
		wstring Name;
	};





	// holds a vector of NamedVariableLists and provides searching functions //
	class NamedVars : public ReferenceCounted, public ThreadSafe{
	public:
		DLLEXPORT NamedVars();

		//! \brief Constructs a NamedVars by stealing variables from another
		//! \note The other object will be empty after this
		DLLEXPORT NamedVars(NamedVars* stealfrom);

		DLLEXPORT NamedVars(const NamedVars &other);
		DLLEXPORT NamedVars(const wstring &datadump);
		DLLEXPORT NamedVars(const vector<shared_ptr<NamedVariableList>> &variables);
		DLLEXPORT NamedVars(shared_ptr<NamedVariableList> variable);
		//! \param takevariable New'd ptr that will be owned by this object
		DLLEXPORT NamedVars(NamedVariableList* takevariable);

		//! \brief Loads a NamedVars object from a packet
		DLLEXPORT NamedVars(sf::Packet &packet);

		DLLEXPORT ~NamedVars();
		// ------------------------------------ //
		DLLEXPORT bool SetValue(const wstring &name, const VariableBlock &value1);
		DLLEXPORT bool SetValue(const wstring &name, VariableBlock* value1);
		DLLEXPORT bool SetValue(const wstring &name, const vector<VariableBlock*> &values);

		DLLEXPORT bool SetValue(NamedVariableList &nameandvalues);

		DLLEXPORT size_t GetValueCount(const wstring &name) const;

		DLLEXPORT VariableBlock& GetValueNonConst(const wstring &name) THROWS;
		DLLEXPORT const VariableBlock* GetValue(const wstring &name) const THROWS;
		DLLEXPORT bool GetValue(const wstring &name, VariableBlock &receiver) const;
		//! \brief Gets a VariableBlock from the specified index on the block matching name
		DLLEXPORT bool GetValue(const wstring &name, const int &index, VariableBlock &receiver) const;
		//! \brief Gets constant value from vector index
		DLLEXPORT bool GetValue(const int &index, VariableBlock &receiver) const;
		DLLEXPORT bool GetValues(const wstring &name, vector<const VariableBlock*> &receiver) const;

		//! \brief Writes this NamedVars to a packet
		DLLEXPORT void AddDataToPacket(sf::Packet &packet) const;

		DLLEXPORT shared_ptr<NamedVariableList> GetValueDirect(const wstring &name) const;

		//! \warning You need to make sure that this is valid while the pointer is used
		DLLEXPORT NamedVariableList* GetValueDirectRaw(const wstring &name) const;


		template<class T>
		DLLEXPORT bool GetValueAndConvertTo(const wstring &name, T &receiver) const{
			// use try block to catch all exceptions (not found and conversion fail //
			try{
				const VariableBlock* tmpblock = this->GetValue(name);
				if(tmpblock == NULL){
					return false;
				}
				if(!tmpblock->ConvertAndAssingToVariable<T>(receiver)){
#ifdef _WIN32
					throw exception("invalid");
#else
					throw bad_exception();
#endif
				}
			}
			catch(...){
				// variable not found / wrong type //
				return false;
			}
			// correct variable has been set //
			return true;
		}

		DLLEXPORT vector<VariableBlock*>* GetValues(const wstring &name) THROWS;

		// Script accessible functions //
		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(NamedVars);

		// Uses the find functions to get first value from the found value //
		// Warning: uses reference counting for return value //
		ScriptSafeVariableBlock* GetScriptCompatibleValue(string name);
		// ------------------------------------ //
		DLLEXPORT int GetVariableType(const wstring &name) const;
		DLLEXPORT int GetVariableType(size_t index) const;
		DLLEXPORT int GetVariableTypeOfAll(const wstring &name) const;
		DLLEXPORT int GetVariableTypeOfAll(size_t index) const;

		DLLEXPORT wstring& GetName(size_t index) THROWS;
		DLLEXPORT bool GetName(size_t index, wstring &name) const;

		DLLEXPORT void SetName(size_t index, const wstring &name);
		DLLEXPORT void SetName(const wstring &oldname, const wstring &name);

		DLLEXPORT bool CompareName(size_t index, const wstring &name) const;
		// ------------------------------------ //
		DLLEXPORT void AddVar(NamedVariableList* newvaluetoadd);
		DLLEXPORT void AddVar(shared_ptr<NamedVariableList> values);
		DLLEXPORT void AddVar(const wstring &name, VariableBlock* valuetosteal);
		DLLEXPORT void Remove(size_t index);
		DLLEXPORT void Remove(const wstring &name);
		//! \brief Removes a value with the given name if it exists
		DLLEXPORT void RemoveIfExists(const wstring &name, ObjectLock &guard);
		// ------------------------------------ //
		DLLEXPORT int LoadVarsFromFile(const wstring &file);

		DLLEXPORT vector<shared_ptr<NamedVariableList>>* GetVec();
		DLLEXPORT void SetVec(vector<shared_ptr<NamedVariableList>> &vec);

		//! \brief Returns the size of the internal variable vector
		DLLEXPORT size_t GetVariableCount() const;

		// ------------------------------------ //

		DLLEXPORT inline int Find(const wstring &name) const{
			GUARD_LOCK_THIS_OBJECT();
			return Find(name, guard);
		}

		DLLEXPORT int Find(const wstring &name, ObjectLock &guard) const;


		// ------------------------------------ //
		template<class T>
		DLLEXPORT bool ShouldAddValueIfNotFoundOrWrongType(const wstring &name){

			int index = Find(name);

			if(index < 0){
				// Add //
				return true;
			}
			// Check is type correct //
			if(!Variables[(size_t)index]->CanAllBeCastedToType<T>()){
				// Incorrect types in the variables //
				return true;
			}

			// No need to do anything //
			return false;
		}

	private:
		std::vector<shared_ptr<NamedVariableList>> Variables;
	};

}

#endif
