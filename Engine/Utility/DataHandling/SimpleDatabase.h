#ifndef LEVIATHAN_SIMPLEDATABASE
#define LEVIATHAN_SIMPLEDATABASE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/DataStoring/DataBlock.h"

namespace Leviathan{

	typedef std::map<wstring, shared_ptr<VariableBlock>> SimpleDatabaseRowObject;
	typedef std::map<wstring, shared_ptr<std::vector<shared_ptr<SimpleDatabaseRowObject>>>> SimpleDatabaseObject;


	//! A class that can be used to pass databases to Rocket and generally keeping simple databases 
	//! \warning Do NOT use this class as non-pointer objects (because linking will fail)
	class SimpleDatabase : public Object{
	public:
		//! \brief Creates a new database. Should be used as pointer
		DLLEXPORT SimpleDatabase(const wstring &databasename);
		DLLEXPORT virtual ~SimpleDatabase();

		// Get functions for getting as strings Rocket compatible get functions //
		DLLEXPORT virtual void GetRow(std::vector<wstring> &row, const wstring &table, int row_index, const std::vector<wstring> &columns);
		DLLEXPORT virtual int GetNumRows(const wstring &table);

		// Search functions //
		DLLEXPORT shared_ptr<VariableBlock> GetValueOnRow(const wstring &table, const wstring &valuekeyname, const VariableBlock &wantedvalue, 
			const wstring &wantedvaluekey);

		// Managing functions //
		DLLEXPORT bool AddValue(const wstring &database, shared_ptr<SimpleDatabaseRowObject> valuenamesandvalues);
		DLLEXPORT bool RemoveValue(const wstring &database, int row);

		// Loading and saving //
		DLLEXPORT bool LoadFromFile(const wstring &file);
		DLLEXPORT void SaveToFile(const wstring &file);

	protected:
		// TODO: implement file saving and loading

		// Makes sure that a table is fine //
		SimpleDatabaseObject::iterator _EnsureTable(const wstring &name);

		// ------------------------------------ //
		// The main database structure //
		SimpleDatabaseObject Database;
	};

}
#endif