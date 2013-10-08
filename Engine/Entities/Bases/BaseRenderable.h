#ifndef LEVIATHAN_BASE_RENDERABLE
#define LEVIATHAN_BASE_RENDERABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "..\GameWorld.h"

namespace Leviathan{

	class BaseRenderable{
	public:
		DLLEXPORT BaseRenderable(bool hidden);
		DLLEXPORT virtual ~BaseRenderable();

		DLLEXPORT virtual bool CheckRender(GraphicalInputEntity* graphics, int mspassed) = 0;

		DLLEXPORT inline bool IsHidden(){
			return Hidden;
		}


	protected:
		bool Hidden;
	};

}
#endif