#pragma once
#ifndef LEVIATHAN_OBJECTFILE
#define LEVIATHAN_OBJECTFILE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ObjectFiles/ObjectFileObject.h"
#include "Common/DataStoring/DataBlock.h"

namespace Leviathan{

	//! Defines the main data structure for an ObjectFile
	//! \warning The user is responsible for locking this class if multi threaded access is desired
	class ObjectFile : public Object{
	public:
		DLLEXPORT ObjectFile();
		DLLEXPORT ~ObjectFile();


		//! \brief Adds a NamedVariableList to this file
		//! \return False when the variable is not added, only when the name is already used
		DLLEXPORT bool AddNamedVariable(shared_ptr<NamedVariableList> var);

		//! \brief Adds a NamedVariableList to this file
		//! \note This is a shorthand for AddNamedVariable defined above
		//! \warning The pointer given will be deleted by this
		//! \see AddNamedVariable
		DLLEXPORT bool AddNamedVariable(NamedVariableList* var);






	protected:

		//! Holds the defined objects
		std::vector<shared_ptr<ObjectFileObject>> DefinedObjects;

		//! Holds all the template definitions
		int TemplateDefinitions;

		//! Holds all objects that are template instantiations
		int TemplateInstantiations;


		//! Holds all the named variables that were in the file
		std::vector<shared_ptr<NamedVariableList>> HeaderVars;


	};

}
#endif