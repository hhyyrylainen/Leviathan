#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "ObjectFiles/ObjectFileList.h"
#include "ObjectFiles/ObjectFileTextBlock.h"
#include "Script/ScriptScript.h"

namespace Leviathan{

	//! \brief Interface for object file objects to implement
	//! \see ObjectFileObjectProper
	class ObjectFileObject{
		friend ObjectFileTemplateDefinition;
	public:
		
		DLLEXPORT virtual ~ObjectFileObject();

		//! \brief Gets the name of this object
		DLLEXPORT virtual const std::string& GetName() const = 0;

		//! \brief Add a variable list to this object
		//! \return False when the name collides
		//! \post The list variable will be empty
		DLLEXPORT virtual bool AddVariableList(std::unique_ptr<ObjectFileList> &list) = 0;


		//! \brief Add a text block to this object
		//! \return False when the name collides
		//! \post The tblock variable will be empty
		DLLEXPORT virtual bool AddTextBlock(std::unique_ptr<ObjectFileTextBlock> &tblock) = 0;


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

	protected:

		ObjectFileObject(){};
	};



	//! \brief Fully defined ObjectFileObject
	//! \see ObjectFileObject
	class ObjectFileObjectProper : public ObjectFileObject{
	public:
		DLLEXPORT ObjectFileObjectProper(const std::string &name, const std::string &typesname,
            std::vector<std::string*> prefix);
        
		DLLEXPORT ~ObjectFileObjectProper();

		DLLEXPORT virtual const std::string& GetName() const;

		DLLEXPORT virtual bool AddVariableList(std::unique_ptr<ObjectFileList> &list);

		DLLEXPORT virtual bool AddTextBlock(std::unique_ptr<ObjectFileTextBlock> &tblock);

		DLLEXPORT virtual void AddScriptScript(std::shared_ptr<ScriptScript> script);

		DLLEXPORT virtual const std::string& GetTypeName() const;

		DLLEXPORT virtual ObjectFileList* GetListWithName(const std::string &name) const;

		DLLEXPORT virtual ObjectFileTextBlock* GetTextBlockWithName(const std::string &name) const;

		DLLEXPORT virtual std::shared_ptr<ScriptScript> GetScript() const;

		DLLEXPORT virtual size_t GetPrefixesCount() const;

		DLLEXPORT virtual const std::string& GetPrefix(size_t index) const;

		DLLEXPORT virtual bool IsThisTemplated() const;

		DLLEXPORT virtual size_t GetListCount() const;

		DLLEXPORT virtual ObjectFileList* GetList(size_t index) const;

		DLLEXPORT virtual size_t GetTextBlockCount() const;

		DLLEXPORT virtual ObjectFileTextBlock* GetTextBlock(size_t index) const;

	protected:

		std::string Name;
		std::string TName;

		std::vector<std::string*> Prefixes;
		std::vector<ObjectFileList*> Contents;
		std::vector<ObjectFileTextBlock*> TextBlocks;

        std::shared_ptr<ScriptScript> Script;
	};



}

