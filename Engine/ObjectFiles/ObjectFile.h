#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "ObjectFiles/ObjectFileObject.h"
#include "Common/DataStoring/DataBlock.h"

namespace Leviathan{

	//! Defines the main data structure for an ObjectFile
	//! \warning The user is responsible for locking this class if multi threaded access is desired
	class ObjectFile{
	public:
		DLLEXPORT ObjectFile();

		//! \brief Constructs a ObjectFile with predefined variables
		//! \note The NamedVariables passed in will be empty after this
		DLLEXPORT ObjectFile(NamedVars &stealfrom);

		DLLEXPORT ~ObjectFile();


		//! \brief Adds a NamedVariableList to this file
		//! \return False when the variable is not added, only when the name is already used
		DLLEXPORT bool AddNamedVariable(std::shared_ptr<NamedVariableList> var);

		//! \brief Adds a NamedVariableList to this file
		//! \note This is a shorthand for AddNamedVariable defined above
		//! \warning The pointer given will be deleted by this
		//! \see AddNamedVariable
		DLLEXPORT FORCE_INLINE bool AddNamedVariable(NamedVariableList* var){
			return AddNamedVariable(std::shared_ptr<NamedVariableList>(var));
		}

		//! \brief Adds a ObjectFileObject to this file
		//! \return True when properly added, false if the name collides
		//! \todo Disallow adding templates with this function
		DLLEXPORT bool AddObject(std::shared_ptr<ObjectFileObject> obj);


		//! \brief Adds a ObjectFileTemplateInstance to this file
		DLLEXPORT void AddTemplateInstance(std::shared_ptr<ObjectFileTemplateInstance> tiobj);

		//! \brief Adds a ObjectFileTemplateInstance to this file
		//! \return True when properly added, false if the name collides
		DLLEXPORT bool AddTemplate(std::shared_ptr<ObjectFileTemplateDefinition> templatedef);


		//! \brief Adds a ObjectFileObject to this file
		//! \warning The pointer will be deleted by this
		//! \see AddObject
		DLLEXPORT FORCE_INLINE bool AddObject(ObjectFileObject* obj){
			return AddObject(std::shared_ptr<ObjectFileObject>(obj));
		}


		//! \brief Returns a raw pointer to HeaderVars
		//! \warning You need to make sure you hold a reference to this to not delete the variables
		DLLEXPORT NamedVars* GetVariables();

		//! \brief Gets the total number of objects (objects + template instances)
		DLLEXPORT size_t GetTotalObjectCount() const;

		//! \brief Gets an ObjectFileObject from an index
		//! \except ExceptionInvalidArgument when the index is out of bounds
		//! \see GetTotalObjectCount
		DLLEXPORT ObjectFileObject* GetObjectFromIndex(size_t index) const;

		//! \brief Gets an ObjectFileObject matching the type name
		//! \note Only the first object is returned matching the type
		//! \todo Add a function which returns all that matched the type
		DLLEXPORT ObjectFileObject* GetObjectWithType(const std::string &typestr) const;

		//! \brief Checks whether the given name is in use
		//! \todo Check template names
		DLLEXPORT bool IsObjectNameInUse(const std::string &name) const;


		//! \brief Instantiates all all templates to actual objects
		//! \return True when all templates had a parent (all names were valid) and no errors occurred
		DLLEXPORT bool GenerateTemplatedObjects();


		//! \brief Finds the template definition matching the name
		//! \return The found object or NULL
		//! \todo Allow template overloading with different number of parameters
		DLLEXPORT std::shared_ptr<ObjectFileTemplateDefinition> FindTemplateDefinition(
            const std::string &name) const;

	protected:

		//! Holds the defined objects
		std::vector<std::shared_ptr<ObjectFileObject>> DefinedObjects;

		//! Holds all the template definitions
		std::vector<std::shared_ptr<ObjectFileTemplateDefinition>> TemplateDefinitions;


		//! Holds all objects that are template instantiations
		std::vector<std::shared_ptr<ObjectFileTemplateInstance>> TemplateInstantiations;


		//! Holds all the named variables that were in the file
		NamedVars HeaderVars;


	};

}

