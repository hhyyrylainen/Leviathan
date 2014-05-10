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

		//! \brief Constructs a ObjectFile with predefined variables
		//! \note The NamedVariables passed in will be empty after this
		DLLEXPORT ObjectFile(NamedVars &stealfrom);

		DLLEXPORT ~ObjectFile();


		//! \brief Adds a NamedVariableList to this file
		//! \return False when the variable is not added, only when the name is already used
		DLLEXPORT bool AddNamedVariable(shared_ptr<NamedVariableList> var);

		//! \brief Adds a NamedVariableList to this file
		//! \note This is a shorthand for AddNamedVariable defined above
		//! \warning The pointer given will be deleted by this
		//! \see AddNamedVariable
		DLLEXPORT bool AddNamedVariable(NamedVariableList* var);

		//! \brief Adds a ObjectFileObject to this file
		//! \return True when properly added, false if the name collides
		DLLEXPORT bool AddObject(shared_ptr<ObjectFileObject> obj);

		//! \brief Adds a ObjectFileObject to this file
		//! \note The pointer will be deleted by this
		//! \see AddObject
		DLLEXPORT bool AddObject(ObjectFileObject* obj);


		//! \brief Returns a raw pointer to HeaderVars
		//! \warning You need to make sure you hold a reference to this to not delete the variables
		DLLEXPORT NamedVars* GetVariables() const;

		//! \brief Gets the total number of objects (objects + template instances)
		DLLEXPORT size_t GetTotalObjectCount() const;

		//! \brief Gets an ObjectFileObject from an index
		//! \except ExceptionInvalidArgument when the index is out of bounds
		//! \see GetTotalObjectCount
		DLLEXPORT ObjectFileObject* GetObjectFromIndex(size_t index) const THROWS;

		//! \brief Gets an ObjectFileObject matching the type name
		//! \note Only the first object is returned matching the type
		//! \todo Add a function which returns all that matched the type
		DLLEXPORT ObjectFileObject* GetObjectWithType(const wstring &typestr) const;


	protected:

		//! Holds the defined objects
		std::vector<shared_ptr<ObjectFileObject>> DefinedObjects;

		//! Holds all the template definitions
		int TemplateDefinitions;

		//! Holds all objects that are template instantiations
		int TemplateInstantiations;


		//! Holds all the named variables that were in the file
		NamedVars HeaderVars;


	};

}
#endif