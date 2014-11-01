#ifndef LEVIATHAN_BASENOTIFIABLEENTITY
#define LEVIATHAN_BASENOTIFIABLEENTITY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/BaseNotifiable.h"
#include "BaseObject.h"


namespace Leviathan{

	class BaseNotifiableEntity : public BaseNotifiable<BaseNotifierEntity, BaseNotifiableEntity>,
                                   public virtual BaseObject
    {
	public:
		DLLEXPORT BaseNotifiableEntity();
		DLLEXPORT virtual ~BaseNotifiableEntity();


	protected:


	};

}
#endif
