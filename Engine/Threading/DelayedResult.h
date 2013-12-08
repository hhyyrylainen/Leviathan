#ifndef LEVIATHAN_DELAYEDRESULT
#define LEVIATHAN_DELAYEDRESULT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	// Used as a return type for functions that don't return immediately //
	class DelayedResult : public Object{
	public:
		DLLEXPORT DelayedResult();
		DLLEXPORT ~DelayedResult();


	private:

	};

}
#endif