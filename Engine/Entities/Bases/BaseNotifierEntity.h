#ifndef LEVIATHAN_BASENOTIFIERENTITY
#define LEVIATHAN_BASENOTIFIERENTITY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/BaseNotifier.h"
#include "BaseObject.h"


namespace Leviathan{

	class BaseNotifierEntity : public BaseNotifier<BaseNotifierEntity, BaseNotifiableEntity>, public virtual BaseObject{
	public:
		DLLEXPORT BaseNotifierEntity();
		DLLEXPORT virtual ~BaseNotifierEntity();


		DLLEXPORT bool SendCustomMessageToChildren(int messagetype, void* data, bool callonall = true);


	protected:

	};

}
#endif