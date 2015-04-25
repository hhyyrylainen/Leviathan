#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/NamedVars.h"

namespace Leviathan{

	//! \brief Interface for object file lists to implement
	//! \see ObjectFileListProper
	class ObjectFileList{
	public:

		DLLEXPORT virtual ~ObjectFileList();

		//! \brief Adds a new variable
		//! \return False when the name conflicts, True otherwise
		DLLEXPORT virtual bool AddVariable(std::shared_ptr<NamedVariableList> var) = 0;


		//! \brief Gets a reference to the underlying variables
		DLLEXPORT virtual NamedVars& GetVariables() = 0;

		//! \brief Gets the name of this list
		DLLEXPORT virtual const std::string& GetName() const = 0;


	protected:

		ObjectFileList(){};


	};


	//! \brief Implementation of ObjectFileList
	//! \see ObjectFileList
	class ObjectFileListProper : public ObjectFileList{
	public:

		DLLEXPORT ObjectFileListProper(const std::string &name);

		DLLEXPORT virtual bool AddVariable(std::shared_ptr<NamedVariableList> var);

		DLLEXPORT virtual NamedVars& GetVariables();

		DLLEXPORT virtual const std::string& GetName() const;



	protected:

		std::string Name;
		NamedVars Variables;

	};

}

