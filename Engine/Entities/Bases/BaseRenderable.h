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

		DLLEXPORT inline bool IsHidden(){
			return Hidden;
		}

		DLLEXPORT void SetHiddenState(bool hidden);

	protected:

		virtual void _OnHiddenStateUpdated() = 0;
		// ------------------------------------ //
		bool Hidden;
	};

}
#endif