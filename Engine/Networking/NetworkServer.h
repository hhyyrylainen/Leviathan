#ifndef LEVIATHAN_NETWORKSERVER
#define LEVIATHAN_NETWORKSERVER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class NetworkServer : public Object{
	public:
		DLLEXPORT NetworkServer();
		DLLEXPORT ~NetworkServer();

		DLLEXPORT void Release();

	private:

	};

}
#endif