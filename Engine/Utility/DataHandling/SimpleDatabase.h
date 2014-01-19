#ifndef LEVIATHAN_SIMPLEDATABASE
#define LEVIATHAN_SIMPLEDATABASE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Rocket/Controls/DataSource.h"
#include "Common/DataStoring/DataBlock.h"

namespace Leviathan{

	typedef std::map<wstring, shared_ptr<VariableBlock>> SimpleDatabaseRowObject;
	typedef std::map<wstring, shared_ptr<std::vector<shared_ptr<SimpleDatabaseRowObject>>>> SimpleDatabaseObject;


	// Class that can be used to pass databases to Rocket //
	// TODO: make a version that doesn't use Rocket
	class SimpleDatabase : public Object, public Rocket::Controls::DataSource{
	public:
		DLLEXPORT SimpleDatabase(const string &rocketname);
		DLLEXPORT virtual ~SimpleDatabase();

		// Rocket compatible get functions //
		DLLEXPORT virtual void GetRow(Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns);
		DLLEXPORT virtual int GetNumRows(const Rocket::Core::String& table);

		// Search functions //
		DLLEXPORT shared_ptr<VariableBlock> GetValueOnRow(const wstring &table, const wstring &valuekeyname, const VariableBlock &wantedvalue, const wstring &wantedvaluekey);

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