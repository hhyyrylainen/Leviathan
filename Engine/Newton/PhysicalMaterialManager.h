#ifndef LEVIATHAN_PHYSICSMATERIALMANAGER
#define LEVIATHAN_PHYSICSMATERIALMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "PhysicalMaterial.h"
#include "NewtonManager.h"

namespace Leviathan{



	class PhysicsMaterialManager{
	public:
		// Newton manager needs to be passed to ensure newton is instantiated //
		DLLEXPORT PhysicsMaterialManager(const NewtonManager* newtoninstanced);
		DLLEXPORT ~PhysicsMaterialManager();

		// This function should be called from the load physical materials callback
		// NOTE: do not delete the ptr after calling this //
		DLLEXPORT void LoadedMaterialAdd(PhysicalMaterial* ptrtotakeownership);

		// TODO: file loading function //

		// ------------------ Material ID fetching functions ------------------ //

		// Gets the ID of a material based on name and NewtonWorld ptr //
		DLLEXPORT int GetMaterialIDForWorld(const wstring &name, NewtonWorld* WorldPtr);

		// Gets the pointer to physical material //
		DLLEXPORT shared_ptr<PhysicalMaterial> GetMaterial(const wstring &name);


		// This function builds the materials for a newton world (this is provided so that you can use multiple threads to setup the game) //
		DLLEXPORT void CreateActualMaterialsForWorld(NewtonWorld* newtonworld);


		DLLEXPORT static PhysicsMaterialManager* Get();
	private:

		// Parts of data construction //
		void _CreatePrimitiveIDList(NewtonWorld* world);
		void _ApplyMaterialProperties(NewtonWorld* world);
		// ------------------------------------ //

		// map for fast finding //
		std::map<wstring, shared_ptr<PhysicalMaterial>> LoadedMaterials;

		static PhysicsMaterialManager* StaticInstance;
	};

}
#endif