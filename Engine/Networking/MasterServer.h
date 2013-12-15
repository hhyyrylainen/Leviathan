#ifndef LEVIATHAN_MASTERSERVER
#define LEVIATHAN_MASTERSERVER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class MasterServer : public Object{
	public:
		DLLEXPORT MasterServer();
		DLLEXPORT ~MasterServer();


		DLLEXPORT void Release();

	private:

	};

}
#endif