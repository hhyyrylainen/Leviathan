#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "PhysicalWorld.h"

namespace Leviathan{

	class NewtonManager{
	public:
		DLLEXPORT NewtonManager();
		DLLEXPORT ~NewtonManager();
		
		// creates a new world that will release itself when no more references //
		DLLEXPORT std::shared_ptr<PhysicalWorld> CreateWorld(GameWorld* owningworld);


		DLLEXPORT static inline NewtonManager* Get(){
			return Staticaccess;
		}
	protected:

		// newton variables //



		// static access //
		static NewtonManager* Staticaccess;
	};

}

