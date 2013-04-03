#ifndef LEVIATHAN_RENDERINGPASSINFO
#define LEVIATHAN_RENDERINGPASSINFO
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{

	class RenderingPassInfo : public Object{
	public:
		DLLEXPORT RenderingPassInfo();
		DLLEXPORT ~RenderingPassInfo();

		DLLEXPORT void ResetState();

	private:

	};

}
#endif