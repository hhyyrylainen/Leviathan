#ifndef LEVIATHAN_SCRIPT_ARGUEMENT
#define LEVIATHAN_SCRIPT_ARGUEMENT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "DataBlock.h"

//#define SCRIPT_ARGUEMENT_TYPE_INT		3
//#define SCRIPT_ARGUEMENT_TYPE_FLOAT		4
//#define SCRIPT_ARGUEMENT_TYPE_BOOL		5
//#define SCRIPT_ARGUEMENT_TYPE_WSTRING	6 // is actually wstring
//#define SCRIPT_ARGUEMENT_TYPE_VOIDPTR	7
namespace Leviathan{

	class ScriptArguement : public Object{
	public:
		DLLEXPORT ScriptArguement::ScriptArguement(DataBlock *data, int type, bool delonrelease);
		DLLEXPORT virtual ScriptArguement::~ScriptArguement();
		DLLEXPORT ScriptArguement::ScriptArguement(const ScriptArguement& arg);
		DLLEXPORT ScriptArguement ScriptArguement::operator=(const ScriptArguement& arg);

		DLLEXPORT operator int();
		DLLEXPORT operator int*();
		DLLEXPORT operator float();
		DLLEXPORT operator float*();
		DLLEXPORT operator bool();
		DLLEXPORT operator bool*();
		DLLEXPORT operator wstring();
		DLLEXPORT operator wstring*();
		DLLEXPORT operator void*();


		DLLEXPORT void SetValue(bool deleteold, DataBlock *data, int type, bool delonrelease);

		int Type;

		DataBlock* Val; // can be pretty much anything
		bool Delete;

	protected:

	};



	class ScriptNamedArguement : public ScriptArguement{
	public:
		DLLEXPORT ScriptNamedArguement::ScriptNamedArguement(const wstring &name, DataBlock* data, int type, bool modifiable, bool delonrelease);
		DLLEXPORT virtual ScriptNamedArguement::~ScriptNamedArguement();
		DLLEXPORT ScriptNamedArguement::ScriptNamedArguement(const ScriptNamedArguement& arg);
		DLLEXPORT ScriptNamedArguement ScriptNamedArguement::operator=(const ScriptNamedArguement& arg);
		DLLEXPORT ScriptNamedArguement ScriptNamedArguement::operator=(const ScriptArguement& arg);

		DLLEXPORT void SetValue(bool deleteold, const wstring &name, DataBlock* data, int type, bool modifiable, bool delonrelease);


		bool Modifiable;
		wstring Name;

	private:

	};
}
#endif