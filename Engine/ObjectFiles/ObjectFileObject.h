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
