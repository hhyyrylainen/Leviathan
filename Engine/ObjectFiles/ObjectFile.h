#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/DataBlock.h"
#include "Common/DataStoring/NamedVars.h"

#include <vector>
#include <string>
#include <memory>


#ifdef LEVIATHAN_USING_ANGELSCRIPT
#include "Script/ScriptScript.h"
#endif // LEVIATHAN_USING_ANGELSCRIPT

namespace Leviathan{

//! \brief Interface for object file lists to implement
//! \see ObjectFileListProper
class ObjectFileList {
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

    ObjectFileList() {
    };


};


//! \brief Implementation of ObjectFileList
//! \see ObjectFileList
class ObjectFileListProper : public ObjectFileList {
public:

    DLLEXPORT ObjectFileListProper(const std::string &name);

    DLLEXPORT virtual bool AddVariable(std::shared_ptr<NamedVariableList> var);

    DLLEXPORT virtual NamedVars& GetVariables();

    DLLEXPORT virtual const std::string& GetName() const;



protected:

    std::string Name;
    NamedVars Variables;

};

//! \brief Interface for object file text blocks to implement
//! \see ObjectFileListProper
class ObjectFileTextBlock {
public:

    DLLEXPORT virtual ~ObjectFileTextBlock();

    //! \brief Adds an UTF8 encoded line
    DLLEXPORT virtual void AddTextLine(const std::string &line) = 0;

    //! \brief Returns the number of text lines
    DLLEXPORT virtual size_t GetLineCount() const = 0;

    //! \brief Gets a line from index
    //! \except ExceptionInvalidArgument when the index is out of bounds
    //! \see GetLineCount
    DLLEXPORT virtual const std::string& GetLine(size_t index) const = 0;

    //! \brief Gets the name of this text block
    DLLEXPORT virtual const std::string& GetName() const = 0;


protected:

    ObjectFileTextBlock() {
    };
};

//! \brief Implementation of ObjectFileTextBlock
//! \see ObjectFileTextBlock
class ObjectFileTextBlockProper : public ObjectFileTextBlock {
public:

    DLLEXPORT ObjectFileTextBlockProper(const std::string &name);
    DLLEXPORT ~ObjectFileTextBlockProper();

    DLLEXPORT virtual void AddTextLine(const std::string &line);

    DLLEXPORT virtual size_t GetLineCount() const;

    DLLEXPORT virtual const std::string& GetLine(size_t index) const;

    DLLEXPORT virtual const std::string& GetName() const;

protected:

    std::string Name;
    std::vector<std::string*> Lines;
};


//! \brief Interface for object file objects to implement
//! \see ObjectFileObjectProper
class ObjectFileObject {
    friend ObjectFileTemplateDefinition;
public:

    DLLEXPORT virtual ~ObjectFileObject();

    //! \brief Gets the name of this object
    DLLEXPORT virtual const std::string& GetName() const = 0;

    //! \brief Add a variable list to this object
    //! \return False when the name collides
    //! \post The list variable will be empty
    DLLEXPORT virtual bool AddVariableList(std::unique_ptr<ObjectFileList>&& list) = 0;


    //! \brief Add a text block to this object
    //! \return False when the name collides
    //! \post The tblock variable will be empty
    DLLEXPORT virtual bool AddTextBlock(std::unique_ptr<ObjectFileTextBlock>&& tblock) = 0;


    //! \brief Add a script block to this object
    //! \note Only the last set ScriptScript will remain
    DLLEXPORT virtual void AddScriptScript(std::shared_ptr<ScriptScript> script) = 0;


    //! \brief Gets the name of the type
    DLLEXPORT virtual const std::string& GetTypeName() const = 0;


    //! \brief Gets an ObjectFileList that matches the name
    //! \return The object if it exists or NULL
    DLLEXPORT virtual ObjectFileList* GetListWithName(const std::string &name) const = 0;

    //! \brief Gets the number of lists in this object
    DLLEXPORT virtual size_t GetListCount() const = 0;

    //! \brief Gets a list from an index
    //! \except ExceptionInvalidArgument when the index is out of bounds
    DLLEXPORT virtual ObjectFileList* GetList(size_t index) const = 0;


    //! \brief Gets an ObjectFileTextBlock that matches the name
    //! \return The object if it exists or NULL
    DLLEXPORT virtual ObjectFileTextBlock* GetTextBlockWithName(const std::string &name)
        const = 0;

    //! \brief Gets the number of text blocks in this object
    DLLEXPORT virtual size_t GetTextBlockCount() const = 0;

    //! \brief Gets a text block from an index
    //! \except ExceptionInvalidArgument when the index is out of bounds
    DLLEXPORT virtual ObjectFileTextBlock* GetTextBlock(size_t index) const = 0;

    //! \brief Returns a std::shared_ptr to our script
    DLLEXPORT virtual std::shared_ptr<ScriptScript> GetScript() const = 0;


    //! \brief Gets the number of prefixes
    DLLEXPORT virtual size_t GetPrefixesCount() const = 0;

    //! \brief Gets a prefix prom an index
    //! \except ExceptionInvalidArgument when the index is out of bounds
    //! \see GetPrefixesCount
    DLLEXPORT virtual const std::string& GetPrefix(size_t index) const = 0;

    //! \brief Returns true when this is a templated object
    //!
    //! This is used while saving to a file to avoid writing template objects
    DLLEXPORT virtual bool IsThisTemplated() const = 0;

    //! \returns A string representation of this object
    DLLEXPORT virtual std::string Serialize(size_t indentspaces = 0) const = 0;

protected:

    ObjectFileObject() {
    };
};



//! \brief Fully defined ObjectFileObject
//! \see ObjectFileObject
class ObjectFileObjectProper : public ObjectFileObject {
public:
    DLLEXPORT ObjectFileObjectProper(const std::string &name, const std::string &typesname,
        std::vector<std::unique_ptr<std::string>>&& prefix);

    DLLEXPORT ~ObjectFileObjectProper();

    DLLEXPORT const std::string& GetName() const override;

    DLLEXPORT bool AddVariableList(std::unique_ptr<ObjectFileList>&& list) override;

    DLLEXPORT bool AddTextBlock(std::unique_ptr<ObjectFileTextBlock>&& tblock) override;

    DLLEXPORT void AddScriptScript(std::shared_ptr<ScriptScript> script) override;

    DLLEXPORT const std::string& GetTypeName() const override;

    DLLEXPORT ObjectFileList* GetListWithName(const std::string &name) const override;

    DLLEXPORT ObjectFileTextBlock* GetTextBlockWithName(const std::string &name) const override;

    DLLEXPORT std::shared_ptr<ScriptScript> GetScript() const override;

    DLLEXPORT size_t GetPrefixesCount() const override;

    DLLEXPORT const std::string& GetPrefix(size_t index) const override;
    DLLEXPORT const std::string* GetPrefixPtr(size_t index) const;
    DLLEXPORT bool IsThisTemplated() const override;

    DLLEXPORT size_t GetListCount() const override;

    DLLEXPORT ObjectFileList* GetList(size_t index) const override;

    DLLEXPORT size_t GetTextBlockCount() const override;

    DLLEXPORT ObjectFileTextBlock* GetTextBlock(size_t index) const override;

    DLLEXPORT std::string Serialize(size_t indentspaces = 0) const override;


protected:

    std::string Name;
    std::string TName;

    std::vector<std::unique_ptr<std::string>> Prefixes;
    std::vector<ObjectFileList*> Contents;
    std::vector<ObjectFileTextBlock*> TextBlocks;

    std::shared_ptr<ScriptScript> Script;
};



//! \brief Class that represents an object created from a template
class ObjectFileTemplateObject : public ObjectFileObjectProper {
public:

    //! \brief Creates an ObjectFileTemplateObject as a wrapper around an ObjectFileObject
    //! \see ObjectFileObjectProper
    DLLEXPORT ObjectFileTemplateObject(const std::string &name, const std::string &typesname,
        std::vector<std::unique_ptr<std::string>>&& prefix);



    DLLEXPORT virtual bool IsThisTemplated() const {

        return true;
    }

};



//! \brief Represents a template instantiation
class ObjectFileTemplateInstance {
    friend ObjectFileTemplateDefinition;
public:

    DLLEXPORT ObjectFileTemplateInstance(const std::string &mastertmplname,
        std::vector<std::unique_ptr<std::string>> &templateargs);


    DLLEXPORT inline const std::string& GetNameOfParentTemplate() const {

        return TemplatesName;
    }

    DLLEXPORT std::string Serialize(size_t indentspaces = 0) const;

protected:

    //! The name of the master template from which the instances are generated
    std::string TemplatesName;

    //! Template arguments
    std::vector<std::unique_ptr<std::string>> Arguments;
};



//! \brief Class that represents a template definition
//! \todo Potentially allow changing the definition to update instantiations
//! \todo Make this more robust and nice and reduce the bloat in the implementation
class ObjectFileTemplateDefinition {
public:

    //! \brief Creates a ObjectFileTemplateDefinition
    //! \warning CreateFromObject might change in the future so please don't use
    //! this function directly
    DLLEXPORT ObjectFileTemplateDefinition(const std::string &name,
        std::vector<std::unique_ptr<std::string>> &parameters,
        std::shared_ptr<ObjectFileObject> obj);

    //! \brief Gets the name of this template
    DLLEXPORT const std::string& GetName() const;


    //! \brief Creates a ObjectFileTemplateDefinition from an ObjectFileObject and a parameter list
    //! \param obj The object from which to construct the template, the pointer will be deleted by this
    DLLEXPORT static std::shared_ptr<ObjectFileTemplateDefinition> CreateFromObject(
        const std::string &name, std::shared_ptr<ObjectFileObject> obj,
        std::vector<std::unique_ptr<std::string>> &templateargs);

    //! \brief Creates an instance from this template
    //! \todo Refactor this function to be smaller
    //! \todo Allow objects to use the special defined values in the ObjectFileProcessor
    DLLEXPORT std::unique_ptr<ObjectFileTemplateObject> CreateInstanceFromThis(
        const ObjectFileTemplateInstance &instanceargs, LErrorReporter* reporterror = nullptr);

    //! \returns A string representation of this object
    DLLEXPORT std::string Serialize() const;

protected:

    void ReplaceStringWithTemplateArguments(std::string &target,
        const std::vector<std::unique_ptr<std::string>> &args);

    std::string ReplaceStringTemplateArguments(const std::string &target,
        const std::vector<std::unique_ptr<std::string>> &args);

    // ------------------------------------ //

    //! Name of this template
    std::string Name;

    //! Template parameter definitions
    std::vector<std::unique_ptr<std::string>> Parameters;

    //! The object from which the instances are created
    std::shared_ptr<ObjectFileObject> RepresentingObject;
};




//! Defines the main data structure for an ObjectFile
//! \warning The user is responsible for locking this class if multi threaded access is desired
class ObjectFile {
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
    DLLEXPORT inline size_t GetTotalObjectCount() const {
        // Add the template objects to actual objects //
        return DefinedObjects.size();
    }

	//! \brief Gets an ObjectFileObject from an index
	//! \except ExceptionInvalidArgument when the index is out of bounds
	//! \see GetTotalObjectCount
	DLLEXPORT ObjectFileObject* GetObjectFromIndex(size_t index) const;

	//! \brief Gets an ObjectFileObject matching the type name
	//! \note Only the first object is returned matching the type
	//! \todo Add a function which returns all that matched the type
	DLLEXPORT ObjectFileObject* GetObjectWithType(const std::string &typestr) const;

    //! \brief Gets all ObjectFileObjects that have the type
    DLLEXPORT std::vector<ObjectFileObject*> GetAllObjectsWithType(const std::string &type) const;

	//! \brief Checks whether the given name is in use
	//! \todo Check template names
	DLLEXPORT bool IsObjectNameInUse(const std::string &name) const;


	//! \brief Instantiates all all templates to actual objects
	//! \return True when all templates had a parent (all names were valid) and no errors occurred
	DLLEXPORT bool GenerateTemplatedObjects(LErrorReporter* reporterror);


	//! \brief Finds the template definition matching the name
	//! \return The found object or NULL
	//! \todo Allow template overloading with different number of parameters
	DLLEXPORT std::shared_ptr<ObjectFileTemplateDefinition> FindTemplateDefinition(
        const std::string &name) const;

    //! \returns The number of template definitions there are available for GetTemplateDefinition
    DLLEXPORT inline size_t GetTemplateDefinitionCount() const {

        return TemplateDefinitions.size();
    }

    //! \returns Template definition if index is valid
    DLLEXPORT inline std::shared_ptr<ObjectFileTemplateDefinition> GetTemplateDefinition(size_t index) {

        if (index >= TemplateDefinitions.size())
            return nullptr;

        return TemplateDefinitions[index];
    }

    //! \returns object (that may be a template instance) definition if index is valid
    DLLEXPORT inline std::shared_ptr<ObjectFileObject> GetObject(size_t index) {

        if (index >= DefinedObjects.size())
            return nullptr;

        return DefinedObjects[index];
    }

    //! \returns The number of template instances there are available for GetTemplateInstance
    DLLEXPORT size_t GetTemplateInstanceCount() const {

        return TemplateInstantiations.size();
    }

    //! \returns Template instance if index is valid
    DLLEXPORT std::shared_ptr<ObjectFileTemplateInstance> GetTemplateInstance(size_t index) {

        if (index >= TemplateInstantiations.size())
            return nullptr;

        return TemplateInstantiations[index];
    }

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

