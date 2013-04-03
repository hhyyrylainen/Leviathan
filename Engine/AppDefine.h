#ifndef LEVIATHAN_APPLICATIONDEFINE
#define LEVIATHAN_APPLICATIONDEFINE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "NamedVars.h"
namespace Leviathan{

	class AppDef{
	public:
		DLLEXPORT AppDef();
		DLLEXPORT AppDef(bool isdef);
		DLLEXPORT AppDef::~AppDef();

		DLLEXPORT NamedVars* GetValues();

		DLLEXPORT static AppDef* GetDefault();


		

	private:
		static AppDef* Defaultconf;
		NamedVars* values;
		

	};
















}
#endif