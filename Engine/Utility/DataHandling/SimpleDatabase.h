#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/DataBlock.h"
#include "Common/ThreadSafe.h"

namespace Leviathan{

	typedef std::map<std::string, std::shared_ptr<VariableBlock>> SimpleDatabaseRowObject;
	typedef std::map<std::string,
                     std::shared_ptr<std::vector<std::shared_ptr<SimpleDatabaseRowObject>>>>
        SimpleDatabaseObject;


	//! A class that can be used to pass databases to Rocket and generally keeping simple databases 
	//! \warning Do NOT use this class as non-pointer objects (because linking will fail)
    //! \todo This needs proper tests and fixing
	class SimpleDatabase : public ThreadSafe{
	public:
		//! \brief Creates a new database. Should be used as pointer
		DLLEXPORT SimpleDatabase(const std::string &databasename);
		DLLEXPORT virtual ~SimpleDatabase();

		// Get functions for getting as strings Rocket compatible get functions //
		DLLEXPORT virtual void GetRow(std::vector<std::string> &row, const std::string &table,
            int row_index, const std::vector<std::string> &columns);
		DLLEXPORT virtual int GetNumRows(const std::string &table);

		// Search functions //
		DLLEXPORT std::shared_ptr<VariableBlock> GetValueOnRow(const std::string &table,
            const std::string &valuekeyname, const VariableBlock &wantedvalue, 
			const std::string &wantedvaluekey);

		// Managing functions //
		DLLEXPORT bool AddValue(const std::string &database,
            std::shared_ptr<SimpleDatabaseRowObject> valuenamesandvalues);
		DLLEXPORT bool RemoveValue(const std::string &database, int row);

		// Loading and saving //
		DLLEXPORT bool LoadFromFile(const std::string &file);
		DLLEXPORT void SaveToFile(const std::string &file);

		//! \brief Serializes a single table to JSON
		//!
		//! The JSON structure creates an object that has a single array member named the same as the table
		//! \return true if the table was found, false otherwise
		DLLEXPORT bool WriteTableToJson(const std::string &tablename, std::string &receiver,
            bool humanreadable = false);

	protected:
		//! \brief Makes sure that a table is fine
		//! \param name Name of the table that is to be created if it doesn't exist
		SimpleDatabaseObject::iterator _EnsureTable(const std::string &name);

		// ------------------------------------ //

		//! The main database structure
		SimpleDatabaseObject Database;
	};

}

