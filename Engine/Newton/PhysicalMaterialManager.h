#ifndef LEVIATHAN_PHYSICSMATERIALMANAGER
#define LEVIATHAN_PHYSICSMATERIALMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class NewtonManager;

	class PhysicsMaterialManager{
	public:
		// Newton manager needs to be passed to ensure newton is instantiated //
		DLLEXPORT PhysicsMaterialManager(const NewtonManager* newtoninstanced);
		DLLEXPORT ~PhysicsMaterialManager();


	private:

	};

}
#endif