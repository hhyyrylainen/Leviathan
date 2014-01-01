#ifndef LEVIATHAN_MASTERSERVERAPPLICATION
#define LEVIATHAN_MASTERSERVERAPPLICATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Application.h"


namespace Leviathan{

	class MasterServerApplication : public LeviathanApplication{
	public:
		DLLEXPORT MasterServerApplication();
		DLLEXPORT ~MasterServerApplication();

		// Overloaded functions to make this program actually a master server //
		DLLEXPORT virtual bool Initialize(AppDef* configuration);
		DLLEXPORT virtual void Release();



	private:

	};

}
#endif