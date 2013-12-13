#ifndef LEVIATHAN_BASE_TICKABLE
#define LEVIATHAN_BASE_TICKABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class BaseTickable /*: public Object these classes are "components" and shouldn't inherit anything */{
	public:
		DLLEXPORT BaseTickable();
        DLLEXPORT virtual ~BaseTickable();

		DLLEXPORT virtual void Tick() = 0;

	protected:
		int TickCount;
	};

}
#endif
