#ifndef LEVIATHAN_NEWTONMANAGER
#define LEVIATHAN_NEWTONMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "PhysicalWorld.h"

#define USE_VISUAL_DEBUGGER

namespace Leviathan{

	class NewtonManager : public Object{
	public:
		DLLEXPORT NewtonManager();
		DLLEXPORT ~NewtonManager();
		
		// creates a new world that will release itself when no more references //
		DLLEXPORT shared_ptr<PhysicalWorld> CreateWorld();


		DLLEXPORT static inline NewtonManager* Get(){
			return Staticaccess;
		}
	protected:

		// newton variables //



		// static access //
		static NewtonManager* Staticaccess;
	};

}
#endif