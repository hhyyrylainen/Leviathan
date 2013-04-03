#ifndef LEVIATHAN_BASE_INITABLE
#define LEVIATHAN_BASE_INITABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class BaseInitable /*: public Object these classes are "components" and shouldn't inherit anything */{
	public:
		DLLEXPORT BaseInitable::BaseInitable();
        DLLEXPORT virtual BaseInitable::~BaseInitable();

		DLLEXPORT virtual bool Init() = 0;
		DLLEXPORT virtual void Release() = 0;


	protected:
		bool Initialized;
	};

}
#endif