#ifndef LEVIATHAN_OBJECTFILEOBJECT
#define LEVIATHAN_OBJECTFILEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ObjectFiles/ObjectFileList.h"
#include "ObjectFiles/ObjectFileTextBlock.h"
#include "Script/ScriptScript.h"

namespace Leviathan{

	//! \brief Interface for object file objects to implement
	//! \see ObjectFileObjectProper
	class ObjectFileObject : public Object{
	public:
		
		DLLEXPORT virtual ~ObjectFileObject();

		//! \brief Gets the name of this object
		DLLEXPORT virtual const wstring& GetName() const = 0;

		//! \brief Add a variable list to this object
		//! \return False when the name collides
		DLLEXPORT virtual bool AddVariableList(shared_ptr<ObjectFileList> list) = 0;


		//! \brief Add a text block to this object
		//! \return False when the name collides
		DLLEXPORT virtual bool AddTextBlock(shared_ptr<ObjectFileTextBlock> tblock) = 0;


		//! \brief Add a script block to this object
		//! \note Only the last set ScriptScript will remain
		DLLEXPORT virtual void AddScriptScript(shared_ptr<ScriptScript> script) = 0;


		//! \brief Gets the name of the type
		DLLEXPORT virtual const wstring& GetTypeName() const = 0;


		//! \brief Gets an ObjectFileList that matches the name
		//! \return The object if it exists or NULL
		DLLEXPORT virtual ObjectFileList* GetListWithName(const wstring &name) const = 0;


		//! \brief Gets an ObjectFileTextBlock that matches the name
		//! \return The object if it exists or NULL
		DLLEXPORT virtual ObjectFileTextBlock* GetTextBlockWithName(const wstring &name) const = 0;

		//! \brief Returns a shared_ptr to our script
		DLLEXPORT virtual shared_ptr<ScriptScript> GetScript() const = 0;


		//! \brief Gets the number of prefixes
		DLLEXPORT virtual size_t GetPrefixesCount() const = 0;

		//! \brief Gets a prefix prom an index
		//! \except ExceptionInvalidArgument when the index is out of bounds
		//! \see GetPrefixesCount
		DLLEXPORT virtual const wstring& GetPrefix(size_t index) const = 0 THROWS;

	protected:

		ObjectFileObject();
	};



	//! \brief Fully defined ObjectFileObject
	class ObjectFileObjectProper : public ObjectFileObject{
	public:
		DLLEXPORT ObjectFileObjectProper(const wstring &name, const wstring &typesname, vector<wstring*> prefix);
		DLLEXPORT ~ObjectFileObjectProper();


	protected:

		wstring Name;
		wstring TName;

		std::vector<wstring*> Prefixes;
		std::vector<ObjectFileList*> Contents;
		std::vector<ObjectFileTextBlock*> TextBlocks;

		shared_ptr<ScriptScript> Script;
	};



}
#endif
