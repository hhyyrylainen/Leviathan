#ifndef LEVIATHAN_OBJECTFILE_LIST
#define LEVIATHAN_OBJECTFILE_LIST
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/DataStoring/NamedVars.h"

namespace Leviathan{

	//! \brief Interface for object file lists to implement
	//! \see ObjectFileListProper
	class ObjectFileList : public Object{
	public:

		DLLEXPORT virtual ~ObjectFileList();

		//! \brief Adds a new variable
		//! \return False when the name conflicts, True otherwise
		DLLEXPORT virtual bool AddVariable(shared_ptr<NamedVariableList> var) = 0;


		//! \brief Gets a reference to the underlying variables
		DLLEXPORT virtual NamedVars& GetVariables() = 0;

		//! \brief Gets the name of this list
		DLLEXPORT virtual const wstring& GetName() const = 0;


	protected:

		ObjectFileList(){};


	};


	//! \brief Implementation of ObjectFileList
	//! \see ObjectFileList
	class ObjectFileListProper : public ObjectFileList{
	public:

		DLLEXPORT ObjectFileListProper(const wstring &name);

		DLLEXPORT virtual bool AddVariable(shared_ptr<NamedVariableList> var);

		DLLEXPORT virtual NamedVars& GetVariables();

		DLLEXPORT virtual const wstring& GetName() const;



	protected:

		wstring Name;
		NamedVars Variables;

	};

}
#endif
