#ifndef LEVIATHAN_SERVERAPPLICATION
#define LEVIATHAN_SERVERAPPLICATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Application.h"

namespace Leviathan{

	class ServerApplication : public LeviathanApplication{
	public:
		DLLEXPORT ServerApplication();
		DLLEXPORT ~ServerApplication();

		// Overloaded functions to make this program actually a server //
		DLLEXPORT virtual bool Initialize(AppDef* configuration);
		DLLEXPORT virtual void Release();

	protected:

	};

}
#endif