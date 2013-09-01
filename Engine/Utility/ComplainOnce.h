#ifndef LEVIATHAN_COMPLAINONCE
#define LEVIATHAN_COMPLAINONCE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{

	class ComplainOnce{
	public:

		DLLEXPORT static bool PrintWarningOnce(const wstring& warning, const wstring& message);
		DLLEXPORT static bool PrintErrorOnce(const wstring& error, const wstring& message);

	private:
		ComplainOnce::ComplainOnce();
		ComplainOnce::~ComplainOnce();

		// fired warnings/errors //
		static vector<wstring*> FiredErrors;
	};

}
#endif