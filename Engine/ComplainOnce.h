#ifndef LEVIATHAN_COMPLAINONCE
#define LEVIATHAN_COMPLAINONCE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "DataStore.h"

namespace Leviathan{

	class ComplainOnce{
	public:

		DLLEXPORT static bool PrintWarningOnce(const wstring& warning, const wstring& message);


	private:
		ComplainOnce::ComplainOnce();
		ComplainOnce::~ComplainOnce();
	};

}
#endif